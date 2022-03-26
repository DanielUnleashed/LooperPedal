#include "AudioPlayer.h"

AudioFile AudioPlayer::audioChannels[MAX_AUDIO_CHANNELS];
hw_timer_t* AudioPlayer::timer;
uint8_t AudioPlayer::channelsUsed = 0;
uint8_t AudioPlayer::longestChannel = 0;

CircularBuffer AudioPlayer::globalBuf;

uint8_t AudioPlayer::playMode = 0; 

SemaphoreHandle_t AudioPlayer::semPlayFreq = NULL;
portMUX_TYPE AudioPlayer::timerMux = portMUX_INITIALIZER_UNLOCKED;

uint32_t AudioPlayer::PLAY_TIME_START = 0;

void AudioPlayer::addAudioFile(char* filePath){
  audioChannels[channelsUsed++].open(filePath);
  debug("Added audio file %s at Channel %d", filePath, channelsUsed-1);
  if(audioChannels[channelsUsed-1].getFileSize() >= audioChannels[longestChannel].getFileSize()){
    longestChannel = channelsUsed-1;
    debug(" (longest)");
  }
  debug("\n");
}

void AudioPlayer::addAudioFile(AudioFile newAudioFile){
  audioChannels[channelsUsed++] = newAudioFile;
}

void AudioPlayer::start(){
  setAllTo(AudioFile::FILE_PLAYING);

  // Audio processing and signal managing task running on core 0
  xTaskCreatePinnedToCore(audioProcessingTask, "AudProcTask", 10000, NULL, 5, NULL, 0); //deleted
  xTaskCreatePinnedToCore(statusMonitorTask, "MonitorTask", 10000, NULL, 1, NULL, 0);
  // Memory task running on core 1
  xTaskCreatePinnedToCore(memoryTask, "MemoryTask", 10000, NULL, 5, NULL, 1);

  delay(10);

  //Start frequency playback interrupt
  dac_output_enable(DAC_CHANNEL_1);
  /*With prescaler of 8, resolution of 0.1 us.*/
  timer = timerBegin(0, 8, true);
  timerAttachInterrupt(timer, frequencyTimer, true);
  timerAlarmWrite(timer, 10000000/PLAY_FREQUENCY, true);
  timerAlarmEnable(timer);
  PLAY_TIME_START = millis();
}

void AudioPlayer::statusMonitorTask(void* funcParams){
  for(;;){
    uint32_t totalPlaybackMillis = audioChannels[longestChannel].getFileSize() * 1000/ PLAY_BYTE_RESOLUTION / PLAY_FREQUENCY;
    uint32_t elapsedPlayback = millis() - PLAY_TIME_START;
    PLAYBACK_TIME pt = Utilities::toPlaybackTimeStruct(elapsedPlayback);
    PLAYBACK_TIME total_pt = Utilities::toPlaybackTimeStruct(totalPlaybackMillis);
    debug("\n*********** CHANNEL STATUS %s/%s ************\n", Utilities::playBackTimeToString(pt), Utilities::playBackTimeToString(total_pt));
    debug("%s\t%-20s%-10s%8s/%-10s\n", "CH.", "FILE NAME", "STATUS", "NOW", "SIZE"); 
    for(uint8_t i = 0; i < channelsUsed; i++){
      AUDIO_FILE_INFO n = audioChannels[i].getAudioFileInfo();
      debug("%02d\t%-20s%-10s%8d/%-10d\n", i, n.fileName, n.state, n.currentFileDirection,n.size); 
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void AudioPlayer::setAllTo(const uint8_t state){
  for(uint8_t i = 0; i < channelsUsed; i++) audioChannels[i].setTo(state);
}

void AudioPlayer::memoryTask(void* funcParams){ 
  for(;;){
    for(uint8_t i = 0; i < channelsUsed; i++){
      audioChannels[i].refreshBuffer();
    }
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void AudioPlayer::audioProcessingTask(void* funcParams){
  for(;;){
    while(globalBuf.getFreeSpace() > BUFFER_REFRESH){
      uint32_t samples = 0;
      for(uint8_t i = 0; i < channelsUsed; i++){
        samples += audioChannels[i].getSample();
      }
      uint16_t finalMix = samples/channelsUsed;
      globalBuf.put(finalMix);
    
      if(audioChannels[longestChannel].hasFileEnded()){
        setAllTo(AudioFile::FILE_PLAYING);
        PLAY_TIME_START = millis();
      }
    }
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void IRAM_ATTR AudioPlayer::frequencyTimer(){
  portENTER_CRITICAL(&timerMux);
  dac_output_voltage(DAC_CHANNEL_1, globalBuf.get());
  portEXIT_CRITICAL(&timerMux);
}

void AudioPlayer::begin(){
  semPlayFreq = xSemaphoreCreateBinary();
  
  playMode = PLAY_ONCE;
  
  if (!SD.begin()) error("Card Mount Failed\n");
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) error("No SD card attached\n");
  
  String cardTypeName = "";
  if (cardType == CARD_MMC) cardTypeName = "MMC";
  else if (cardType == CARD_SD) cardTypeName = "SDSC";
  else if (cardType == CARD_SDHC) cardTypeName = "SDHC";
  else cardTypeName = "UNKNOWN";
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  debug("SD Card Type: %s. SD Card Size: %lluMB\n", cardTypeName, cardSize);
}

void AudioPlayer::debug(const char* x, ...){
  if(!DEBUG_AUDIOPLAYER_MESSAGES) return;
  va_list args;
  va_start(args, x);
  Utilities::debug(x, args);
  va_end(args);
}

void AudioPlayer::error(const char* x, ...) {
  va_list args;
  va_start(args, x);
  Utilities::error(x, args);
  va_end(args);
}
