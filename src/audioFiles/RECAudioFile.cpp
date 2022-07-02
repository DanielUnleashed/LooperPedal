#include "RECAudioFile.h"

RECAudioFile::RECAudioFile(){}

RECAudioFile::RECAudioFile(bool channel, ADC* inputADC){
    ID = REC_FILE_ID;
    adcChannel = channel;
    adc = inputADC;

    recordingName = "ump";

    generateFileName(recordingCount);

    SD.mkdir("/rec");
    currentRecording = SD.open(fileLoc, FILE_WRITE);
}

uint16_t RECAudioFile::getSample(){
    if(recordingCount == 0) return 0x8000;

    fileDirectionToBuffer += 2;
    if(recordingCount>0 && fileDirectionToBuffer >= fileSize){
        fileDirectionToBuffer = 0;
        // Stop the recording! 
        stopRecording();
    }
    return buf.get();
}

void RECAudioFile::refreshBuffer(){
    // If stopRecording() has been called, the recording that has been stored into currentRecording
    // will be copied to recBuf[0] and the other recordings (recBuf[0 and 1]) will be mixed and stored
    // into recBuf[1].
    if(stopRecordingFlag) generateNewRECLayer();

    // While recording, save to the SD file the stored ADC buffer.
    if(isRecording) writeToFile();

    // Refresh the buffers of the last recording made.
    if(recordingCount > 0) readFromSD(0);

    // Refresh the buffers of the mixed iterations before the last recording.
    if(recordingCount > 1) mixFromSD();

    if(buf.getFreeSpace() < BUFFER_REFRESH) return;
 
    uint8_t mixedChannels = (recordingCount > 0 && playLastRecording) + (recordingCount>1);
    for(uint16_t i = 0; i < BUFFER_REFRESH; i++){
        if(mixedChannels == 0){
            buf.put(0x8000);
        }else{
            uint32_t mix = 0;
            if(recordingCount > 0 && playLastRecording) mix += recBuf[0].get();
            if(recordingCount > 1) mix += recBuf[1].get();

            buf.put(mix/mixedChannels);
        }
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
    
    recBuf[channel].put(&recFiles[channel], buffSize);
}

void RECAudioFile::mixFromSD(){
    if(recBuf[1].getFreeSpace() < BUFFER_REFRESH) return;
    if(laterPrevHasBeenMixed) return readFromSD(1);

    recFiles[1].seek(fileDirectionToBuffer);
    recFiles[2].seek(fileDirectionToBuffer);

    uint32_t remainingBytes = fileSize - fileDirectionToBuffer;
    uint16_t buffSize = BUFFER_REFRESH<<1;
    if(remainingBytes < buffSize){
        buffSize = remainingBytes;
        laterPrevHasBeenMixed = true;
    }
    uint8_t bufAData[buffSize];
    uint8_t bufBData[buffSize];
    recFiles[1].read(bufAData, buffSize);
    recFiles[2].read(bufBData, buffSize);

    for(uint16_t i = 0; i < buffSize; i+=2){
        uint16_t chA = bufAData[i] | (bufAData[i+1] << 8);
        uint16_t chB = bufBData[i] | (bufBData[i+1] << 8);
        uint16_t mix = ((uint32_t)(chA+chB)) >> 1;
        recBuf[1].put(mix);
    }

    recBuf[1].copyToFile(&tempMixingChannel, buffSize);

    if(laterPrevHasBeenMixed){
        recFiles[2].close();
        
        String fname = tempMixingChannel.name();
        String path = tempMixingChannel.path();
        if(!fname.startsWith("/") && !path.endsWith("/")) {
            path += "/";
        }
        path += fname;
        recFiles[1].close();
        recFiles[1] = SD.open(path, FILE_READ);
    }
}

void RECAudioFile::writeToFile(){
    uint16_t totalW = adc->getSavedReadingsCount(adcChannel);
    if(totalW < BUFFER_REFRESH) return;
    
    bool endOfFileFlag = false;
    if(recordingCount>0 && fileDirectionToBuffer + totalW > fileSize){
        totalW = fileSize - fileDirectionToBuffer;
        endOfFileFlag = true;
    }

    adc->getLastReadings(/*Channel = */0).copyToFile(&currentRecording, totalW<<1);

    if(endOfFileFlag){
        stopRecording();
    }
}

void RECAudioFile::startRecording(){
    isRecording = true;
    if(recordingCount > 0){
        currentRecording.seek(fileDirectionToBuffer);
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

    recFiles[2] = recFiles[1];
    recFiles[1] = recFiles[0];
    recFiles[0] = SD.open(fileLoc, FILE_READ);
    if(recordingCount == 0){
        fileSize = recFiles[0].size();
        Utilities::debug("REC size: %d\n", fileSize);
    }

    if(recordingCount > 1){
        tempMixingChannel = SD.open(generateFileName(recordingCount-2, recordingCount-1), FILE_WRITE);
        laterPrevHasBeenMixed = false;
    }

    stopRecordingFlag = false;
    Serial.println("Stopped recording\n");
    recordingCount++;
}

String RECAudioFile::generateFileName(uint8_t number){
    char newFileName[30];
    snprintf(newFileName, 30, "%s%s_%03d.raw", REC_FOLDER, recordingName.c_str(), number);
    fileLoc = String(newFileName);
    return fileLoc;
}

String RECAudioFile::generateFileName(uint8_t chA, uint8_t chB){
    char newFileName[35];
    snprintf(newFileName, 35, "%s%s_MIX_%03d-%03d.raw", REC_FOLDER, fileLoc.c_str(), chA, chB);
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