#include "AudioPlayer.h"

uint32_t AudioPlayer::PLAY_TIME_START = 0;

AudioFile AudioPlayer::audioChannels[MAX_AUDIO_CHANNELS];
hw_timer_t* AudioPlayer::timer;
uint8_t AudioPlayer::channelsUsed = 0;
uint8_t AudioPlayer::longestChannel = 0;
CircularBuffer AudioPlayer::globalBuf;
uint8_t AudioPlayer::playMode = 0; 
bool AudioPlayer::isPlaying = false;
portMUX_TYPE AudioPlayer::timerMux = portMUX_INITIALIZER_UNLOCKED;
DAC AudioPlayer::dac(CS_DAC);
ADC AudioPlayer::adc(CS_ADC);
TaskHandle_t AudioPlayer::audioProcessingTaskHandle;
TaskHandle_t AudioPlayer::statusMonitorTaskHandle;
TaskHandle_t AudioPlayer::memoryTaskHandle;

void IRAM_ATTR AudioPlayer::ISR_BUTTON_1(){
  if(isPlaying) pause();
  else play();
}

void IRAM_ATTR AudioPlayer::ISR_BUTTON_2(){
  
}

void IRAM_ATTR AudioPlayer::ISR_BUTTON_3(){
  
}

void IRAM_ATTR AudioPlayer::ISR_BUTTON_4(){
  
}

void AudioPlayer::begin(){
  pinMode(PUSH_BUTTON_1, INPUT);
  pinMode(PUSH_BUTTON_2, INPUT);
  pinMode(PUSH_BUTTON_3, INPUT);
  pinMode(PUSH_BUTTON_4, INPUT);

  attachInterrupt(PUSH_BUTTON_1, ISR_BUTTON_1, RISING);
  attachInterrupt(PUSH_BUTTON_2, ISR_BUTTON_2, RISING);
  attachInterrupt(PUSH_BUTTON_3, ISR_BUTTON_3, RISING);
  attachInterrupt(PUSH_BUTTON_4, ISR_BUTTON_4, RISING);

  SDBoot();
  dac.begin();
  adc.begin();

  // Audio processing and signal managing task running on core 0
  xTaskCreatePinnedToCore(audioProcessingTask, "AudProcTask", 10000, NULL, 5, &audioProcessingTaskHandle, 0);
  xTaskCreatePinnedToCore(statusMonitorTask, "MonitorTask", 10000, NULL, 1, &statusMonitorTaskHandle, 0);
  // Memory task running on core 1
  xTaskCreatePinnedToCore(memoryTask, "MemoryTask", 10000, NULL, 5, &memoryTaskHandle, 1);

  vTaskSuspend(audioProcessingTaskHandle);
  vTaskSuspend(statusMonitorTaskHandle);
  vTaskSuspend(memoryTaskHandle);

  delay(10);

  //Start frequency playback interrupt
  /*With prescaler of 8, resolution of 0.1 us.*/
  timer = timerBegin(0, 8, true);
  timerAttachInterrupt(timer, frequencyTimer, true);
  timerAlarmWrite(timer, 10000000/PLAY_FREQUENCY, true);

  isPlaying = false;
  longestChannel = 0;
}

void AudioPlayer::play(){
  vTaskResume(audioProcessingTaskHandle);
  vTaskResume(statusMonitorTaskHandle);
  vTaskResume(memoryTaskHandle);
  PLAY_TIME_START = millis();
  isPlaying = true;
  setAllTo(AudioFile::FILE_PLAYING);
  timerAlarmEnable(timer);
  Serial.println("-Resumed.");
}

void AudioPlayer::pause(){
  timerAlarmDisable(timer);
  vTaskSuspend(audioProcessingTaskHandle);
  vTaskSuspend(memoryTaskHandle);
  isPlaying = false;
  setAllTo(AudioFile::FILE_PAUSED);
  Serial.println("-Paused.");
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
    while(globalBuf.getFreeSpace() > BUFFER_REFRESH && isPlaying){
      uint32_t samples = 0;
      for(uint8_t i = 0; i < channelsUsed; i++){
        samples += audioChannels[i].getSample() >> 4;
      }
      uint16_t finalMix = samples/channelsUsed;
      if(!globalBuf.put(finalMix)) break;
    
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
  //uint16_t rec = adc.read(0);
  dac.writeFromISR(globalBuf.get());
  portEXIT_CRITICAL(&timerMux);
}

void AudioPlayer::addAudioFile(char* filePath){
  if(!audioChannels[channelsUsed].open(filePath)) Utilities::error("Fatal fail opening file %s", filePath);
  Utilities::debug("Added audio file %s at Channel %d", audioChannels[channelsUsed].fileName.c_str(), channelsUsed);
  if(audioChannels[channelsUsed].getFileSize() >= audioChannels[longestChannel].getFileSize()){
    longestChannel = channelsUsed;
    Utilities::debug(" (longest)");
  }
  channelsUsed++;
  Utilities::debug("\n");
}

void AudioPlayer::addAudioFile(AudioFile newAudioFile){
  audioChannels[channelsUsed++] = newAudioFile;
}

void AudioPlayer::SDBoot(){  
  playMode = PLAY_ONCE;
  
  if (!SD.begin()) Utilities::error("Card Mount Failed\n");
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) Utilities::error("No SD card attached\n");
  
  String cardTypeName = "";
  if (cardType == CARD_MMC) cardTypeName = "MMC";
  else if (cardType == CARD_SD) cardTypeName = "SDSC";
  else if (cardType == CARD_SDHC) cardTypeName = "SDHC";
  else cardTypeName = "UNKNOWN";
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Utilities::debug("SD Card Type: %s. SD Card Size: %lluMB\n", cardTypeName, cardSize);
}

void AudioPlayer::statusMonitorTask(void* funcParams){
  for(;;){
    if(isPlaying && channelsUsed>0){
      uint32_t totalPlaybackMillis = audioChannels[longestChannel].getFileSize() * 1000/ 2 / PLAY_FREQUENCY;
      uint32_t elapsedPlayback = millis() - PLAY_TIME_START;
      PLAYBACK_TIME pt = Utilities::toPlaybackTimeStruct(elapsedPlayback);
      PLAYBACK_TIME total_pt = Utilities::toPlaybackTimeStruct(totalPlaybackMillis);
      Utilities::debug("\n*********** CHANNEL STATUS %s/%s ************\n", Utilities::playBackTimeToString(pt), Utilities::playBackTimeToString(total_pt));
      Utilities::debug("%s\t%-50s%-10s%-6s%-6s  %8s/%-10s\n", "CH.", "FILE NAME", "STATUS", "RES.", "PROG.", "NOW", "SIZE"); 
      for(uint8_t i = 0; i < channelsUsed; i++){
        AUDIO_FILE_INFO n = audioChannels[i].getAudioFileInfo();
        Utilities::debug("%02d\t%-50s%-10s%-6d%-6d%% %8d/%-10d\n", i, n.fileName, n.state, n.bitRes, n.progress, n.currentFileDirection, n.size); 
      }
    }else{
      const char* stat;
      if(!isPlaying){
        stat = "PAUSED";
        vTaskSuspend(NULL);
      }else if(channelsUsed == 0){
        stat = "NO CHANNELS ADDED";
      }else{
        stat = "UNKNOWN";
      }
      Utilities::debug("\n*********** CHANNEL STATUS: %s ************\n", stat);
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void AudioPlayer::setAllTo(const uint8_t state){
  for(uint8_t i = 0; i < channelsUsed; i++) audioChannels[i].setTo(state);
}