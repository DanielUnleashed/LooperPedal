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
    .fileName = fileLoc.c_str(),
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

String AudioFile::getFileName(){
  uint16_t lastBarFound = 0; //Finds the last /
  for(uint16_t i = fileLoc.length()-1; i > 0; i--){
    if(fileLoc.charAt(i) == '/'){
      lastBarFound = i;
      break;
    }
  }
  return fileLoc.substring(lastBarFound, fileLoc.length());
}

String AudioFile::getFileLocation(){
  return fileLoc;
}