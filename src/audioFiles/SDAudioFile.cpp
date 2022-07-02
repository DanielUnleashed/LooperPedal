#include "SDAudioFile.h"

const String SDAudioFile::PROCESSED_FOLDER = "/proc";

SDAudioFile::SDAudioFile(){
  ID = SD_FILE_ID;
}

bool SDAudioFile::open(char *filePath){
  fileStatus = FILE_OPENING;
  fileLoc = String(filePath);

  if(!fetchSDAudioFileData()) return false;
  refreshBuffer();
  fileStatus = FILE_READY;
  return true;
}

bool SDAudioFile::hasFileEnded(){
  return fileStatus == FILE_ENDED;
}

void SDAudioFile::calculateTotalIteration(uint32_t maxFileSize){
  maxIterations = maxFileSize/fileSize;
}

uint16_t SDAudioFile::getSample(){
  /* If the end of the song is reached inside the circular buffer,
     then the file will be tagged as 'ended' and so, if in the next iteration 
     it remains tagged as such, it will return silence.   
  */
  if(buf.getReadIndex() == finalReadIndexOfFile){
    if(currentIteration < maxIterations){
      fileStatus = FILE_ENDED;
      currentIteration = 0;
    }else{
      // Same fileStatus.
      currentIteration++;
    }
    finalReadIndexOfFile = 0xFFFF;
    return buf.get();
  }

  if(fileStatus == FILE_PLAYING) return buf.get();
  //else return 0x80; // For unsigned 8 bit audio.
  else return 0x8000; // For unsigned 16 bit audio.
}

void SDAudioFile::refreshBuffer(){
  // If this audio file has its buffer with content, then, there's no need to refreshen it.
  if(buf.getFreeSpace() < BUFFER_REFRESH) return;
  
  dataFile.seek(fileDirectionToBuffer);
  
  uint32_t remainingBytes = fileSize - fileDirectionToBuffer;
  uint16_t buffSize = BUFFER_REFRESH<<1;
  if(remainingBytes < buffSize){
    buffSize = remainingBytes;
    fileDirectionToBuffer = 0;
  }else{
    fileDirectionToBuffer += buffSize;
  }
  uint8_t bufData[buffSize];
  dataFile.read(bufData, buffSize);

  buf.put(bufData, buffSize);

  if(buffSize != (BUFFER_REFRESH<<1)) finalReadIndexOfFile = buf.getWriteIndex();
}

bool SDAudioFile::fetchSDAudioFileData(){
  if(fileLoc.endsWith(".wav")){
    WavFile wavFile(fileLoc);
    WAV_FILE_INFO wavInfo = wavFile.processToRawFile();
    fileLoc = String(wavInfo.fileName);
    //fileSize = wavInfo.dataSize;
  } else if(fileLoc.endsWith(".raw")){
    // Nothing at the moment.
  }else{
    error("Filetype not suported!");
    return false;
  }

  // Must be sure it is a .raw file.
  dataFile = SD.open(fileLoc, FILE_READ);
  fileSize = dataFile.size();

  if(fileSize == 0){
    debug("File %s is empty!\n", fileLoc.c_str());
    return false;
  }

  fileDirectionToBuffer = 0;
  //Byte res. = audioRes/8 (+ 1 if audioRes%8 > 0)
  byteAudioResolution = (audioResolution>>3) + ((audioResolution & 0x07)>0);
  debug("Loaded %s (size %d bytes) loaded! [AUDIO RESOLUTION: %d (%d bytes)]\n", fileLoc.c_str(), fileSize, audioResolution, byteAudioResolution);
  return true;
}

String SDAudioFile::getStatusString(){
  String status = "";
  switch (fileStatus) {
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

void SDAudioFile::debug(const char* x, ... ) {
  if(!DEBUG_FILE_MESSAGES) return;
  va_list args;
  va_start(args, x);
  Utilities::debug(x, args);
  va_end(args);
}

void SDAudioFile::error(const char* x, ... ) {
  va_list args;
  va_start(args, x);
  Utilities::error(x, args);
  va_end(args);
  for(;;){}
}
