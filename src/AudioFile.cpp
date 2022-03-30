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
  /* If it's the case that inside the circular buffer it's the end of the song
     then, the file will be tagged as 'ended' and so, in the next iteration, if
     it reamains as such state, it will return silence.   
  */
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
  //double freqRatio = audioFrequency/PLAY_FREQUENCY;
  dataFile.seek(fileDirectionToBuffer);
  if(buf.getFreeSpace() >= BUFFER_REFRESH){
    for(uint16_t i = 0; i < BUFFER_REFRESH; i++){
      //For signed 16 bit audio.
      uint32_t mixBuff = 0; //For multiple channel audio to mono
      for(uint8_t j = 0; j < channelNumber; j++){
        uint16_t data = read16();
        data += 0x8000; //For converting from signed 16 bit int to uint16.
        mixBuff += data;
      }
      mixBuff /= channelNumber;

      uint16_t bufData = 0;
      if(mixBuff > 0xFFFF) bufData = 0xFFFF; //To prevent clipping
      else bufData = mixBuff;

      buf.put(bufData);

      /*if(isOversampled){
        fileDirectionToBuffer = byteAudioResolution*channelNumber*round(sampleIndex*freqRatio);
        sampleIndex++;
      }else{
        fileDirectionToBuffer += 2*channelNumber;
      }*/

      fileDirectionToBuffer += 2*channelNumber;

      // For unsigned 8 bit audio.
      /*uint8_t data = dataFile.read();
      buf.put(data);
      fileDirectionToBuffer++;*/

      if(fileDirectionToBuffer >= fileSize){
        if(fileType == WAV_FILE) fileDirectionToBuffer = 44;
        else fileDirectionToBuffer = 0;

        dataFile.seek(fileDirectionToBuffer);
        sampleIndex = 0;
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
    audioFrequency = read32();

    dataFile.seek(22);
    channelNumber = read16();

    dataFile.seek(34);
    audioResolution = read16();

    dataFile.seek(40);
    fileSize = read32();

    fileDirectionToBuffer = 44; //Skip the RIFF header.
  }else{
    fileType = RAW_FILE;
    audioResolution = 8;
    audioFrequency = 8000;
    fileSize = dataFile.size();

    fileDirectionToBuffer = 0;
  }

  //Byte res. = audioRes/8 (+ 1 if audioRes%8 > 0)
  byteAudioResolution = (audioResolution>>3) + ((audioResolution & 0x07)>0);
  isOversampled = audioFrequency!=PLAY_FREQUENCY;
  debug("Loaded %s (size %d bytes) loaded! [AUDIO RESOLUTION: %d (%d bytes)]\n", fileName, fileSize, audioResolution, byteAudioResolution);
}

uint16_t AudioFile::read16(){
     return dataFile.read() | (dataFile.read() << 8);
}

uint32_t AudioFile::read32(){
  uint32_t ans = 0;
  for(uint8_t i = 0; i < 4; i++){
    ans |= dataFile.read()<<(8*i);
  }
  return ans;
}

uint32_t AudioFile::getFileSize(){
  return fileSize;
}

AUDIO_FILE_INFO AudioFile::getAudioFileInfo(){
  AUDIO_FILE_INFO ret = {
    .fileName = fileName,
    .currentFileDirection = fileDirectionToBuffer,
    .size = fileSize,
    .progress = (fileDirectionToBuffer*100UL)/fileSize,
    .state = getStatusString(),
    .frequency = audioFrequency,
    .bitRes = audioResolution,
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
