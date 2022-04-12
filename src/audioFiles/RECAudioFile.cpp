#include "RECAudioFile.h"

RECAudioFile::RECAudioFile(){}

RECAudioFile::RECAudioFile(bool channel, ADC* inputADC){
    ID = REC_FILE_ID;
    adcChannel = channel;
    adc = inputADC;
    fileName = "r";

    SD.mkdir("/rec");
    currentRecording = SD.open(generateFileName(recordingCount), FILE_WRITE);
}

uint16_t RECAudioFile::getSample(){
    if(recordingCount == 0) return 0x8000;
    fileDirectionToBuffer += 2;
    if(recordingCount>0 && fileDirectionToBuffer >= fileSize) fileDirectionToBuffer = 0;
    return buf.get();
}

void RECAudioFile::refreshBuffer(){
    if(stopRecordingFlag) generateNewRECLayer();
    // Will save diferent files for each recording. 
    // Then it will mix them later, that's made so that the last change can be 
    // undone and later done.
    if(isRecording) writeToFile();

    if(recordingCount > 0) readFromSD(0);
    
    if(recordingCount > 1) mixFromSD(1);

    if(buf.getFreeSpace() < BUFFER_REFRESH) return;

    for(uint16_t i = 0; i < BUFFER_REFRESH; i++){
        uint32_t mix = 0;
        uint8_t mixedChannels = 0;

        if(recordingCount > 0){
            if(playLastRecording){
                mix += recBuf[0].get();
                mixedChannels++;
            }
        }
        if(recordingCount > 1){
            mix += recBuf[1].get();
            mixedChannels++;
        }
        
        if(mixedChannels == 0) buf.put(0x8000);
        else buf.put(mix/mixedChannels);
    }
}

void RECAudioFile::readFromSD(uint8_t channel){
    uint16_t freeBufferSpace = recBuf[channel].getFreeSpace();
    if(freeBufferSpace < BUFFER_REFRESH) return;
    readFromSD(channel, fileDirectionToBuffer, freeBufferSpace);
}

void RECAudioFile::readFromSD(uint8_t channel, uint32_t dir, uint16_t dataLength){
    recFiles[channel].seek(dir);
    uint32_t remainingBytes = fileSize - fileDirectionToBuffer;
    uint16_t buffSize = dataLength<<1;
    if(remainingBytes < buffSize) buffSize = remainingBytes;
    uint8_t bufData[buffSize];
    recFiles[channel].read(bufData, buffSize);

    //Serial.printf("rem: %d,  buffs: %d", remainingBytes, buffSize);
    
    recBuf[channel].put(bufData, buffSize);

    //if(buffSize != dataLength<<1) readFromSD(channel, 0, dataLength-(buffSize>>1));
}

void RECAudioFile::mixFromSD(uint8_t channel){
    if(recBuf[channel].getFreeSpace() < BUFFER_REFRESH) return;
    if(laterPrevHasBeenMixed) return readFromSD(1);

    recFiles[channel].seek(fileDirectionToBuffer);

    uint32_t remainingBytes = fileSize - fileDirectionToBuffer;
    uint16_t buffSize = BUFFER_REFRESH<<1;
    if(remainingBytes < buffSize){
        buffSize = remainingBytes;
        laterPrevHasBeenMixed = true;
    }
    uint8_t bufAData[buffSize];
    uint8_t bufBData[buffSize];
    recFiles[channel].read(bufAData, buffSize);
    recFiles[channel+1].read(bufBData, buffSize);

    for(uint16_t i = 0; i < buffSize; i+=2){
        uint16_t chA = bufAData[i] | (bufAData[i+1] << 8);
        uint16_t chB = bufBData[i] | (bufBData[i+1] << 8);
        uint16_t mix = ((uint32_t)(chA+chB)) >> 1;
        recBuf[channel].put(mix);
    }

    uint16_t toWriteBuff[buffSize>>1];
    recBuf[channel].get(toWriteBuff, buffSize>>1);
    tempMixingChannel.write((uint8_t*)toWriteBuff, buffSize);
}

void RECAudioFile::writeToFile(){
    uint16_t totalW = adc->getSavedReadingsCount(adcChannel);
    if(totalW < BUFFER_REFRESH) return;
    
    bool endOfFileFlag = false;
    if(recordingCount>0 && fileDirectionToBuffer + totalW > fileSize){
        totalW = fileSize - fileDirectionToBuffer;
        endOfFileFlag = true;
    }

    bool loopEndedFlag = false;
    if(recordingCount>0 && (fileDirectionToBuffer + totalW)>=recordingStartingDirection){
        totalW = recordingStartingDirection - fileDirectionToBuffer;
        loopEndedFlag = true;
    }

    uint16_t arr[totalW];
    adc->getLastReadings(adcChannel, arr, totalW);
    currentRecording.write((uint8_t*)arr, totalW<<1);

    if(loopEndedFlag){
        generateNewRECLayer();
        return;
    }
    if(endOfFileFlag) currentRecording.seek(0);
}

void RECAudioFile::startRecording(){
    isRecording = true;
    if(recordingCount > 0){
        currentRecording.seek(fileDirectionToBuffer);
        recordingStartingDirection = fileDirectionToBuffer;
    }
    // Delete the last recording.
    if(!playLastRecording) recordingCount--;
    Serial.println("Now recording...");
}

void RECAudioFile::stopRecording(){
    stopRecordingFlag = true;
}

void RECAudioFile::generateNewRECLayer(){
    isRecording = false;
    
    currentRecording.close(); 

    recFiles[1] = recFiles[0];
    recFiles[0] = SD.open(generateFileName(recordingCount), FILE_READ);
    if(recordingCount == 0){
        fileSize = recFiles[0].size();
        Utilities::debug("REC size: %d\n", fileSize);
    }
    
    recordingCount++;
    currentRecording = SD.open(generateFileName(recordingCount), FILE_WRITE);

    if(recordingCount > 2){
        tempMixingChannel = SD.open(generateFileName(recordingCount-2, recordingCount-1), FILE_WRITE);
        laterPrevHasBeenMixed = false;
    } 
    stopRecordingFlag = false;
    Serial.println("Stopped recording\n");
}

String RECAudioFile::generateFileName(uint8_t number){
    char newFileName[30];
    snprintf(newFileName, 30, "%s%s_%03d.raw", REC_FOLDER, fileName.c_str(), number);
    return String(newFileName);
}

String RECAudioFile::generateFileName(uint8_t chA, uint8_t chB){
    char newFileName[35];
    snprintf(newFileName, 35, "%s%s_MIX_%03d-%03d.raw", REC_FOLDER, fileName.c_str(), chA, chB);
    return String(newFileName);
}

void RECAudioFile::undoRedoLastRecording(){
    if(recordingCount == 0) return;
    playLastRecording = !playLastRecording;
}

String RECAudioFile::getStatusString(){
    return "TODO";
}

bool RECAudioFile::hasFileEnded(){
    return fileDirectionToBuffer == 0;
}