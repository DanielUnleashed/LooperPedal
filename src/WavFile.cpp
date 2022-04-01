#include "WavFile.h"

WavFile::WavFile(File f){
    wavFile = f;
    getHeader();
}

File WavFile::processToRawFile(){
    SD.mkdir("/proc");
    String fileName = String(wavFile.name());
    fileName = fileName.substring(0, fileName.length()-4);
    fileName.concat(".raw");
    String fileLoc = "/proc";
    fileLoc.concat(fileName);

    File transformedFile = SD.open(fileLoc, FILE_WRITE); //Must be also opened to read!
    if(transformedFile.size() > 0){
        wavFile.close();
        return transformedFile;
    }else{
        Utilities::debug("File %s exists, but is empty!\n", fileLoc.c_str());
    }

    Utilities::debug("Parsing %s\n", fileLoc.c_str());
    processAudioData(transformedFile);
    wavFile.close();
    return transformedFile;
}

void WavFile::getHeader(){
    WAV_HEADER wavHeader = {
        //TODO: This could be done better...
        .RIFF_ID = (char*)"RIFF",
        .RIFF_DataSize = read32(),
        .RIFF_TYPE_ID = (char*)"WAVE",
        .fmt_ID = (char*)"fmt ",
        .fmt_DataSize = read32(),
        .fmt_FormatTag = read16(),
        .channelNum = read16(),
        .sampleRate = read32(),
        .byteRate = read32(),
        .blockAlign = read16(),
        .bitsPerSample = read16(),
        .data_ID = (char*)"data",
        .data_DataSize = read32(),
    };
    wavInfo = wavHeader;
}

bool WavFile::processAudioData(File outFile){
    if(wavInfo.sampleRate == PLAY_FREQUENCY && wavInfo.channelNum == 1) return directCopy(outFile);

    double freqRatio = wavInfo.sampleRate/PLAY_FREQUENCY;
    uint32_t sampleIndex = 0;
    uint32_t fileDir = 40;
    uint32_t topIndex = wavInfo.data_DataSize/wavInfo.channelNum/2;
    Serial.printf("TopIndex: %d\n", topIndex);

    uint16_t it = 0;
    while(sampleIndex < topIndex){
        Serial.printf("It.: %d\n", it);
        uint16_t dataToBuffer[1024];
        uint16_t currIndex;
        for(currIndex = 0; currIndex < 1024; currIndex++){
            if(!wavFile.seek(fileDir+40)) break;

            uint32_t mixBuff = 0; //For multiple channel audio to mono
            for(uint8_t j = 0; j < wavInfo.channelNum; j++){
                uint16_t data = read16();
                data += 0x8000; //For converting from signed 16 bit int to uint16.
                mixBuff += data;
            }
            mixBuff /= wavInfo.channelNum;

            uint16_t bufData = 0;
            if(mixBuff > 0xFFFF) bufData = 0xFFFF; //To prevent clipping
            else bufData = mixBuff;
            
            dataToBuffer[currIndex] = bufData;

            if(wavInfo.sampleRate != PLAY_FREQUENCY){        
                fileDir = 2*wavInfo.channelNum*round(sampleIndex*freqRatio);
            }else{
                fileDir += 2*wavInfo.channelNum;
            }
            sampleIndex++;
        }

        for(uint16_t i = 0; i < currIndex; i++)
            outFile.write((uint8_t *)&dataToBuffer[i],2);

        outFile.flush();
        it++;
    }
    return true;
}

bool WavFile::directCopy(File outFile){
    Utilities::debug("File can be directly copied!");
    wavFile.seek(40);
    uint32_t startTime = millis();

    const uint16_t dataBufferSize = 512;
    uint16_t totalIt = wavFile.size()/dataBufferSize/2;
    uint32_t dir = 40;
    uint16_t it = 0;

    while(true){
        uint16_t bufLength = dataBufferSize;
        if((dir + dataBufferSize*2) > wavFile.size()) bufLength = (wavFile.size() - dir)/2;
        else dir += dataBufferSize*2;

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

        if(bufLength != dataBufferSize) break;
    }

    uint32_t ellapsed = millis() - startTime;
    Serial.printf("That took %d ms\n", ellapsed);
    outFile.flush();
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