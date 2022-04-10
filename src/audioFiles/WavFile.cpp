#include "WavFile.h"

WavFile::WavFile(String fileLoc){
    wavLoc = fileLoc;

    if(!SD.exists(fileLoc)){
        Utilities::error("File '%s' doesn't exist!\n", fileLoc.c_str());
        return;
    }

    wavFile = SD.open(fileLoc, FILE_READ);
    if (!wavFile) {
        Utilities::error("Failed to open file '%s' for reading/writing\n", fileLoc.c_str());
        return;
    }

    wavFileDataSize = wavFile.size();
    if(wavFileDataSize == 0){
        Utilities::error("File %s is empty!\n", fileLoc.c_str());
        return;
    }
    
    getHeader();
}

WAV_FILE_INFO WavFile::processToRawFile(){
    SD.mkdir("/proc");
    String fileName  = wavLoc.substring(0, wavLoc.length()-4);
    fileName.concat(PLAY_FREQUENCY);
    fileName.concat(".raw");
    String procFolder = "/proc";
    procFolder.concat(fileName);
    
    String procFile = String(procFolder);

    if(SD.exists(procFile)){
        File transformedFile = SD.open(procFile, FILE_READ);
        uint32_t transformedFileSize = transformedFile.size();
        if(transformedFileSize > 0){ //Maybe do something more sophisticated like CRC (?)
            wavFile.close();
            transformedFile.close();
            WAV_FILE_INFO ret = {procFile, transformedFileSize};
            return ret;
        }else{
            Utilities::debug("File %s exists, but is empty!\n", procFile.c_str());
        }
        transformedFile.close();
    }

    File transformedFile = SD.open(procFile, FILE_WRITE);

    Utilities::debug("File %s (%d bytes) will be parsed into %s\n", wavLoc.c_str(), wavFileDataSize, procFile.c_str());
    if(!processAudioData(transformedFile)) Utilities::error("Something happened parsing %s\n", procFile.c_str());
    
    wavFile.close();
    transformedFile.flush();
    uint32_t transformedFileSize = transformedFile.size();
    transformedFile.close();
    WAV_FILE_INFO ret = {procFile, transformedFileSize};
    return ret;
}

void WavFile::getHeader(){
    WAV_HEADER wavHeader = {
        //TODO: This could be done better...
        .RIFF_ID = readBytes(4),
        .RIFF_DataSize = read32(),
        .RIFF_TYPE_ID = readBytes(4),
        .fmt_ID = readBytes(4),
        .fmt_DataSize = read32(),
        .fmt_FormatTag = read16(),
        .channelNum = read16(),
        .sampleRate = read32(),
        .byteRate = read32(),
        .blockAlign = read16(),
        .bitsPerSample = read16(),
        .data_ID = readBytes(4),
        .data_DataSize = read32(),
    };
    wavInfo = wavHeader;
}

//TODO: Add support for lower frequencies.
bool WavFile::processAudioData(File outFile){
    if(wavInfo.sampleRate == PLAY_FREQUENCY && wavInfo.channelNum == 1) return directCopy(outFile);

    Serial.printf("Input freq.: %d,  Channels: %d\n", wavInfo.sampleRate, wavInfo.channelNum);

    double freqRatio = ((double)wavInfo.sampleRate)/PLAY_FREQUENCY;

    Serial.printf("Freq. ratio = %.4f\n", freqRatio);

    uint16_t maxProcessedBufferSize = ceil(DATA_COPY_BUFFER_SIZE/freqRatio);
    uint32_t startTime = millis();

    uint32_t totalIt = wavFileDataSize/DATA_COPY_BUFFER_SIZE/2;
    uint32_t dir = 40;
    uint32_t it = 0;

    uint16_t startingPosition = 0;

    wavFile.seek(40);

    while(true){
        uint16_t bufLength = DATA_COPY_BUFFER_SIZE;
        if((dir + DATA_COPY_BUFFER_SIZE*2) > wavFileDataSize) bufLength = (wavFileDataSize - dir)/2;
        else dir += DATA_COPY_BUFFER_SIZE*2;

        uint16_t dataToBuffer[bufLength];
        uint16_t processedBuffer[maxProcessedBufferSize];
        uint16_t realProcBuffLength = 0;

        wavFile.read((uint8_t*)&dataToBuffer, bufLength*2);
        for(realProcBuffLength= 0; realProcBuffLength < maxProcessedBufferSize; realProcBuffLength++){
            // Sampling section
            uint16_t bufferIndex = (uint16_t) (round(realProcBuffLength*freqRatio))*wavInfo.channelNum + startingPosition;
            //Serial.printf("buffInd: %d, ind: %d, start: %d", bufferIndex, realProcBuffLength, startingPosition);
            //delay(100);
            if(bufferIndex >= bufLength){
                // startingPosition shifts the bufferIndex so that when in a previous loop the next
                // sample is out of reach, it can be reached in the next iteration.
                startingPosition = bufferIndex - bufLength;
                break;
            }

            uint32_t mixBuff = 0; //For multiple channel audio to mono
            uint8_t j = 0;
            for(j = 0; j < wavInfo.channelNum; j++){
                if(bufferIndex + j >= bufLength){
                    startingPosition++; 
                    break; // Will take only one sample in this case. Oh well... 
                } 
                mixBuff += (uint16_t)(dataToBuffer[bufferIndex+j] + 0x8000);
            }
            //Average the channels.
            mixBuff /= (j+1);

            uint16_t bufData = 0;
            if(mixBuff > 0xFFFF) bufData = 0xFFFF; //To prevent clipping
            else bufData = mixBuff;

            processedBuffer[realProcBuffLength] = bufData;
        }

        if(outFile.write((uint8_t*)&processedBuffer, realProcBuffLength*2) != realProcBuffLength*2){
            Serial.println("Write failed!");
            break;
        }

        if(it % 100 == 0) Utilities::debug("Progress: %d %%\n", it*100/totalIt);
        it++;

        if(bufLength != DATA_COPY_BUFFER_SIZE) break;
    }

    uint32_t ellapsed = millis() - startTime;
    double speed = ((double)wavFileDataSize)/ellapsed;  
    Serial.printf("That took %d ms (%0.2f KB/s)\n", ellapsed, speed);
    return true;
}

bool WavFile::directCopy(File outFile){
    Utilities::debug("File can be directly copied!\n");
    wavFile.seek(40);
    uint32_t startTime = millis();

    uint32_t totalIt = wavFileDataSize/DATA_COPY_BUFFER_SIZE/2;
    uint32_t dir = 40;
    uint32_t it = 0;

    while(true){
        uint16_t bufLength = DATA_COPY_BUFFER_SIZE;
        if((dir + DATA_COPY_BUFFER_SIZE*2) > wavFileDataSize) bufLength = (wavFileDataSize - dir)/2;
        else dir += DATA_COPY_BUFFER_SIZE*2;

        uint16_t dataToBuffer[bufLength];
        wavFile.read((uint8_t*)&dataToBuffer, bufLength*2);
        for(uint16_t i = 0; i < bufLength; i++){
            dataToBuffer[i] += 0x8000; //For converting from signed 16 bit int to uint16.
        }

        if(outFile.write((uint8_t*)&dataToBuffer, bufLength*2) != bufLength*2){
            Serial.println("Write failed!");
            break;
        }

        if(it % 100 == 0) Serial.printf("Progress: %d %%\n", it*100/totalIt);
        it++;

        if(bufLength != DATA_COPY_BUFFER_SIZE) break;
    }

    uint32_t ellapsed = millis() - startTime;
    double speed = ((double)wavFileDataSize)/ellapsed;  
    Serial.printf("That took %d ms (%0.2f KB/s)\n", ellapsed, speed);
    return true;
}

//TODO: change so a WAV_HEADER can be used.
void WavFile::printHeader(File outFile){
    //Write header.
    outFile.seek(0);
    outFile.write((const uint8_t*)"RIFF", 4);   // Chunk ID
    uint32_t outFileSize = wavInfo.sampleRate * 2;
    uint32_t chunkDataSize = outFileSize + 36;            
    outFile.write((uint8_t*)&chunkDataSize, 4); // Chunk data size 
    outFile.write((const uint8_t*)"WAVE", 4);   // RIFF Type ID
    outFile.write((const uint8_t*)"fmt ", 4);   // Chunk1 ID 
    const uint8_t arr1[] = {0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00};
    outFile.write(arr1, sizeof(arr1));          // Chunk1 data size + format tag + num channels
    uint32_t freq = 8000;
    outFile.write((const uint8_t*)&freq, 4); 
    uint32_t byteRate = freq*2;
    outFile.write((uint8_t*)&byteRate, 4);
    uint16_t blockAlign = 2;
    outFile.write((uint8_t*)&blockAlign, 2);
    uint16_t bitsPerSample= 2;
    outFile.write((uint8_t*)&bitsPerSample, 2);
    outFile.write((const uint8_t*)"data", 4); // Chunk1 ID 
    outFile.write((uint8_t*)&outFileSize, 4);      // Chunk data size 
    outFile.close();
}

uint8_t* WavFile::readBytes(uint8_t length){
    uint8_t* ret;
    ret = (uint8_t*) malloc(4);
    wavFile.read(ret, 4);
    return ret;
}

uint16_t WavFile::read16(){
    uint16_t ret = 0;
    wavFile.read((uint8_t*) &ret, 2);
    return ret;
}

uint32_t WavFile::read32(){
  uint32_t ans = 0;
  for(uint8_t i = 0; i < 4; i++){
    ans |= wavFile.read()<<(8*i);
  }
  return ans;
}