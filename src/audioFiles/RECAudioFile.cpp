#include "RECAudioFile.h"

RECAudioFile::RECAudioFile(bool channel, ADC* inputADC){
    ID = REC_FILE_ID;
    adcChannel = channel;
    adc = inputADC;
    currentRecording = SD.open("/rec/0.raw", FILE_WRITE);
}

uint16_t RECAudioFile::getSample(){
    if(isRecording){
        uint16_t inputData = adc -> read(adcChannel);
        buf.put(inputData);
    }

    fileDirectionToBuffer += 2;

    uint32_t mix = 0;
    if(currentRecording == 1){
        if(playLastRecording){
            mix = recBuf[0].get();
        }
    }else if(currentRecording > 1){
        mix = recBuf[1].get();
        if(playLastRecording){
            mix += recBuf[0].get();
            mix = mix >> 1;
        }
    }
    return mix;
}

void RECAudioFile::refreshBuffer(){
    // Will save diferent files for each recording. 
    // Then it will mix them later, that's made so that the last change can be 
    // undone and later done.
    if(isRecording && buf.getWrittenSpace() > BUFFER_REFRESH) writeToFile();

    if(recordingCount == 1){
        readFromSD(0);
    }else if(recordingCount > 1){
        readFromSD(0);
        mixFromSD(1);
    }
}

void RECAudioFile::readFromSD(uint8_t channel){
    if(recBuf[channel].getFreeSpace() < BUFFER_REFRESH) return;

    recFiles[channel].seek(fileDirectionToBuffer);

    uint32_t remainingBytes = fileSize - fileDirectionToBuffer;
    uint16_t buffSize = BUFFER_REFRESH<<1;
    if(remainingBytes < buffSize) buffSize = remainingBytes;
    uint8_t bufData[buffSize];
    recFiles[channel].read(bufData, buffSize);
    
    for(uint16_t i = 0; i < buffSize; i+=2)
        recBuf[channel].put(bufData[i] | (bufData[i+1] << 8));
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
    uint16_t totalW = buf.getWrittenSpace();
    uint16_t arr[totalW];
    buf.get(arr, totalW);
    currentRecording.write((uint8_t*)arr, totalW>>1);
    currentRecording.flush();
}

void RECAudioFile::startRecording(){
    isRecording = true;
    // Delete the last recording.
    if(!playLastRecording) recordingCount--;
}

void RECAudioFile::stopRecording(){
    isRecording = false;

    currentRecording.close();

    recFiles[1] = recFiles[0];
    recFiles[0] = SD.open(fileName, FILE_READ);
    
    recordingCount++;
    currentRecording = SD.open(generateFileName(recordingCount), FILE_WRITE);

    if(recordingCount > 2){
        tempMixingChannel = SD.open(generateFileName(recordingCount-2, recordingCount-1), FILE_WRITE);
        laterPrevHasBeenMixed = false;
    } 
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
    laterPrevHasBeenMixed = !laterPrevHasBeenMixed;
}

String RECAudioFile::getStatusString(){
    return "TODO";
}

bool RECAudioFile::hasFileEnded(){
    return fileDirectionToBuffer == 0;
}