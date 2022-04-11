#include "AudioFile.h"

void AudioFile::setStatus(uint8_t stat){
    fileStatus = stat;   
}

uint8_t AudioFile::getStatus(){
    return fileStatus;
}

uint32_t AudioFile::getFileSize(){
    return fileSize;
}

uint32_t AudioFile::getCurrentFileDirection(){
    return fileDirectionToBuffer;
}

AUDIO_FILE_INFO AudioFile::getAudioFileInfo(){
  AUDIO_FILE_INFO ret = {
    .fileName = fileName.c_str(),
    .currentFileDirection = fileDirectionToBuffer,
    .size = fileSize,
    .progress = 0,
    .state = getStatusString(),
    .bitRes = audioResolution,
  };
  if(fileSize != 0) ret.progress = (fileDirectionToBuffer*100UL)/fileSize;
  return ret;
}

bool AudioFile::is(uint8_t askID){
  return ID == askID;
}