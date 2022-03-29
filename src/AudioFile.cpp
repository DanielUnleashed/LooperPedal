#include "AudioFile.h"

AudioFile::AudioFile(){}

void AudioFile::open(char *filePath){
  fileState = FILE_OPENING;

  fileName = (char*)malloc(sizeof(filePath));
  fileName = filePath;

  dataFile = SD.open(filePath, FILE_READ);
  if (!dataFile) {
    error("Failed to open file for reading/writing\n");
  }

  if(dataFile.size() == 0){
    error("File %s is empty!\n", filePath);
  }

  fetchAudioFileData();
  refreshBuffer();
  fileState = FILE_READY;
}

void AudioFile::setTo(const uint8_t state){
  fileState = state;
}

bool AudioFile::hasFileEnded(){
  return fileState == FILE_ENDED;
}

uint16_t AudioFile::getSample(){
  if(buf.getReadIndex() == finalReadIndexOfFile){
    fileState = FILE_ENDED;
    finalReadIndexOfFile = 0xFFFF;
    return buf.get();
  }

  if(fileState == FILE_PLAYING) return buf.get();
  //else return 0x80; // For unsigned 8 bit audio.
  else return 0x8000; // For unsigned 16 bit audio.
}

void AudioFile::refreshBuffer(){
  if(buf.getFreeSpace() >= BUFFER_REFRESH){
    dataFile.seek(fileDirectionToBuffer);
    for(uint16_t i = 0; i < BUFFER_REFRESH; i++){
      // For unsigned 16 bit audio (little indian). 
      uint16_t data = dataFile.read();
      data |= dataFile.read()<<8;
      data += 0x8000;
      buf.put(data);
      fileDirectionToBuffer+=2;

      // For unsigned 8 bit audio.
      /*uint8_t data = dataFile.read();
      buf.put(data);
      fileDirectionToBuffer++;*/

      if(fileDirectionToBuffer >= fileSize){
        fileDirectionToBuffer = 0;
        dataFile.seek(0);
        finalReadIndexOfFile = buf.getWriteIndex();
      }
    }
  }
}

void AudioFile::fetchAudioFileData(){
  String str = String(fileName);
  if(str.endsWith(".wav")){
    fileType = WAV_FILE;
    
    dataFile.seek(24);
    for(uint8_t i = 0; i < 4; i++){
      audioFrequency = (audioFrequency<<8) | dataFile.read();
    }
    dataFile.seek(34);
    for(uint8_t i = 0; i < 2; i++){
      audioResolution = (audioResolution<<8) | dataFile.read();
    }
    dataFile.seek(40);
    for(uint8_t i = 0; i < 4; i++){
      fileSize = (fileSize<<8) | dataFile.read();
    }
  }else{
    fileType = RAW_FILE;
    audioResolution = 8;
    audioFrequency = 8000;
    fileSize = dataFile.size();
  }

  byteAudioResolution = audioResolution>>3;
  if((audioResolution & 0x07) > 0) byteAudioResolution++; // Same as audioResolution % 8
  
  debug("Loaded %s (size %d bytes) loaded! [AUDIO RESOLUTION: %d (%d bytes)]\n", fileName, fileSize, audioResolution, byteAudioResolution);
}

uint32_t AudioFile::getFileSize(){
  return fileSize;
}

AUDIO_FILE_INFO AudioFile::getAudioFileInfo(){
  AUDIO_FILE_INFO ret = {
    .fileName = String(fileName),
    .currentFileDirection = fileDirectionToBuffer,
    .size = fileSize,
    .state = getStatusString(),
  };
  return ret;
}

String AudioFile::getStatusString(){
  String status = "";
  switch (fileState) {
  case FILE_OPENING:
    status = "OPENING";
    break;
  case FILE_READY:
    status = "READY";
    break;
  case FILE_ENDED:
    status = "ENDED";
    break;
  case FILE_PAUSED:
    status = "PAUSED";
    break;
  case FILE_PLAYING:
    status = "PLAYING";
    break;
  default:
    status = "UNKNOWN";
    break;
  }
  return status;
}

void AudioFile::debug(const char* x, ... ) {
  if(!DEBUG_FILE_MESSAGES) return;
  va_list args;
  va_start(args, x);
  Utilities::debug(x, args);
  va_end(args);
}

void AudioFile::error(const char* x, ... ) {
  va_list args;
  va_start(args, x);
  Utilities::error(x, args);
  va_end(args);
  for(;;){}
}
