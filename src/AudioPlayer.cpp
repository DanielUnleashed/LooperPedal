#include "AudioPlayer.h"

AudioFile* AudioPlayer::audioChannels[MAX_TOTAL_CHANNELS];
uint8_t AudioPlayer::channelsUsed = 0;
hw_timer_t* AudioPlayer::timer;

uint8_t AudioPlayer::SDChannelPos[MAX_AUDIO_CHANNELS];
uint8_t AudioPlayer::RECChannelPos[MAX_REC_CHANNELS];
uint8_t AudioPlayer::audioChannelsUsed = 0;
uint8_t AudioPlayer::recChannelsUsed = 0;

uint8_t AudioPlayer::longestChannel = 0;
CircularBuffer AudioPlayer::globalBuf;
uint8_t AudioPlayer::playMode = 0; 
bool AudioPlayer::isPlaying = false;
bool AudioPlayer::isRecording = false;
portMUX_TYPE AudioPlayer::timerMux = portMUX_INITIALIZER_UNLOCKED;
DAC AudioPlayer::dac(CS_DAC);
ADC AudioPlayer::adc(CS_ADC);
TaskHandle_t AudioPlayer::audioProcessingTaskHandle;
TaskHandle_t AudioPlayer::statusMonitorTaskHandle;
TaskHandle_t AudioPlayer::memoryTaskHandle;

DebounceButton AudioPlayer::button1(PUSH_BUTTON_1);
DebounceButton AudioPlayer::button2(PUSH_BUTTON_2);
DebounceButton AudioPlayer::button3(PUSH_BUTTON_3);
DebounceButton AudioPlayer::button4(PUSH_BUTTON_4);

void IRAM_ATTR AudioPlayer::ISR_BUTTON_1(){
  if(!button1.clicked()) return;

  if(isPlaying) pause();
  else play();
}

void IRAM_ATTR AudioPlayer::ISR_BUTTON_2(){
  if(!button2.clicked()) return;

  for(uint8_t i = 0; i < recChannelsUsed; i++){
    RECAudioFile* currCh = getRECAudioFile(i);
    if(isRecording) currCh -> stopRecording();
    else currCh -> startRecording();
  }
  isRecording = !isRecording;
  if(!isPlaying) play();
}

void IRAM_ATTR AudioPlayer::ISR_BUTTON_3(){
  if(!button3.clicked()) return;

  for(uint8_t i = 0; i < recChannelsUsed; i++)
    getRECAudioFile(i) -> undoRedoLastRecording();
}

void IRAM_ATTR AudioPlayer::ISR_BUTTON_4(){}

void AudioPlayer::begin(){
  attachInterrupt(PUSH_BUTTON_1, ISR_BUTTON_1, CHANGE);
  attachInterrupt(PUSH_BUTTON_2, ISR_BUTTON_2, CHANGE);
  attachInterrupt(PUSH_BUTTON_3, ISR_BUTTON_3, CHANGE);
  attachInterrupt(PUSH_BUTTON_4, ISR_BUTTON_4, CHANGE);

  SDBoot();
  dac.begin();
  adc.begin();

  delay(10);

#ifdef LAUNCH_CONSOLE
  // Audio processing and signal managing task running on core 0
  xTaskCreatePinnedToCore(statusMonitorTask, "MonitorTask", 10000, NULL, 1, &statusMonitorTaskHandle, 0);
  vTaskSuspend(statusMonitorTaskHandle);
 #endif 

  // Memory task running on core 1
  xTaskCreatePinnedToCore(memoryTask, "MemoryTask", 10000, NULL, 5, &memoryTaskHandle, 1);
  vTaskSuspend(memoryTaskHandle);

  //Start frequency playback interrupt
  /*With prescaler of 8, resolution of 0.1 us.*/
  timer = timerBegin(0, 8, true);
  timerAttachInterrupt(timer, frequencyTimer, true);
  timerAlarmWrite(timer, 10000000/PLAY_FREQUENCY, true);

  isPlaying = false;
  longestChannel = 0;
}

void AudioPlayer::play(){
  isPlaying = true;
  vTaskResume(memoryTaskHandle);
#ifdef LAUNCH_CONSOLE
  vTaskResume(statusMonitorTaskHandle);
#endif
  setAllTo(SD_FILE_ID, AudioFile::FILE_PLAYING);
  timerAlarmEnable(timer);
}

void AudioPlayer::pause(){
  timerAlarmDisable(timer);
  vTaskSuspend(memoryTaskHandle);
  isPlaying = false;
  setAllTo(SD_FILE_ID, AudioFile::FILE_PAUSED);
}

void AudioPlayer::memoryTask(void* funcParams){ 
  /*uint32_t average = 0;
  uint16_t it = 0;
  uint32_t min = 0xFF;
  uint32_t max = 0;*/
  for(;;){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if(!isPlaying || channelsUsed == 0) continue;

    //uint32_t start = micros();

    for(uint8_t i = 0; i < channelsUsed; i++) audioChannels[i]->refreshBuffer();

    uint16_t totalIt = globalBuf.getFreeSpace();
    for(uint16_t i = 0; i < totalIt; i++){
      uint32_t samples = 0;
      for(uint8_t i = 0; i < channelsUsed; i++){
        samples += audioChannels[i]->getSample() >> 4;
      }
      uint16_t finalMix = samples/channelsUsed;
      globalBuf.put(finalMix);
    
      if(audioChannels[longestChannel]->hasFileEnded()){
        setAllTo(SD_FILE_ID, AudioFile::FILE_PLAYING);
      }
    }
    /*uint32_t ellapsed = micros() - start;
    average += ellapsed;
    it++;
    if(ellapsed > max) max = ellapsed;
    else if(ellapsed < min) min = ellapsed;
    if(it == 0xFF){
      average = average>>8;
      Serial.printf("\nAv: %d, min: %d, max: %d\n", average, min, max);
      average = 0;
      it = 0;
      min = 0xFFFF;
      max = 0;
    }*/
  }
  vTaskDelete(NULL);
}

void IRAM_ATTR AudioPlayer::frequencyTimer(){
  vTaskEnterCritical(&timerMux);
#ifdef PASS_AUDIO_INPUT_DURING_RECORDING
  uint16_t adcRead = adc.updateReadings();
  uint32_t mix;
  if(isRecording) mix = (globalBuf.get() + adcRead)>>1;
  else mix = globalBuf.get();
  dac.writeFromISR(mix);
#else
  //adc.updateReadings();
  dac.writeFromISR(globalBuf.get());
#endif

  if(globalBuf.getFreeSpace() > BUFFER_REFRESH){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(memoryTaskHandle, &xHigherPriorityTaskWoken);
  }
  vTaskExitCritical(&timerMux);
}

void AudioPlayer::statusMonitorTask(void* funcParams){
  for(;;){
    if(isPlaying && channelsUsed>0){
      uint32_t totalPlaybackMillis = audioChannels[longestChannel]->getFileSize() * 1000/ 2 / PLAY_FREQUENCY;
      uint32_t elapsedPlayback = audioChannels[longestChannel]->getCurrentFileDirection() * 1000/ 2 / PLAY_FREQUENCY;
      PLAYBACK_TIME pt = Utilities::toPlaybackTimeStruct(elapsedPlayback);
      PLAYBACK_TIME total_pt = Utilities::toPlaybackTimeStruct(totalPlaybackMillis);
      debug("\n********************** CHANNEL STATUS %s/%s ***********************\n", Utilities::playBackTimeToString(pt), Utilities::playBackTimeToString(total_pt));
      debug("%s\t%-30s%-10s%-6s%-6s  %8s/%-10s\n", "CH.", "FILE NAME", "STATUS", "RES.", "PROG.", "NOW", "SIZE"); 
      for(uint8_t i = 0; i < channelsUsed; i++){
        AUDIO_FILE_INFO n = audioChannels[i]->getAudioFileInfo();
        debug("%02d%s\t%-30s%-10s%-6d%-6d%% %8d/%-10d\n", i, i == longestChannel ? "*" : "", n.fileName, n.state, n.bitRes, n.progress, n.currentFileDirection, n.size); 
      }
    }else{
      String stat;
      if(!isPlaying){
        stat = "PAUSED";
        vTaskSuspend(NULL);
      }else if(channelsUsed == 0){
        stat = "NO CH. ADDED";
      }else{
        stat = "UNKNOWN";
      }
      debug("\n*********** CHANNEL STATUS: %s ************\n", stat.c_str());
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void AudioPlayer::setAllTo(const uint8_t audioFileID, const uint8_t state){
  for(uint8_t i = 0; i < audioChannelsUsed; i++)
    getSDAudioFile(i) -> setStatus(state);
}

SDAudioFile* AudioPlayer::getSDAudioFile(uint8_t index){
  return (SDAudioFile*) audioChannels[SDChannelPos[index]];
}

RECAudioFile* AudioPlayer::getRECAudioFile(uint8_t index){
  return (RECAudioFile*) audioChannels[RECChannelPos[index]];
}

void AudioPlayer::addSDAudioFile(char* filePath){
  if(channelsUsed == MAX_AUDIO_CHANNELS){
    debug("Reached max. number of channels!\n");
    return;
  }

  SDAudioFile* audFile = new SDAudioFile;
  audioChannels[channelsUsed] = audFile;
  if(!audFile -> open(filePath)) error("Fatal fail opening file %s", filePath);
  debug("File %s added at Channel %d", audioChannels[channelsUsed]->fileName.c_str(), channelsUsed);
  if(audioChannels[channelsUsed]->getFileSize() >= audioChannels[longestChannel]->getFileSize()){
    longestChannel = channelsUsed;
    debug(" (longest)");
  }
  debug("\n");

  SDChannelPos[audioChannelsUsed++] = channelsUsed++;
  uint32_t longestChannelSize = audioChannels[longestChannel]->getFileSize();
  for(uint8_t i = 0; i < channelsUsed; i++){
    if(audioChannels[i] -> ID == SD_FILE_ID)
      ((SDAudioFile*)audioChannels[i]) -> calculateTotalIteration(longestChannelSize);
  }
}

void AudioPlayer::addRECAudioFile(bool channel){
  RECAudioFile* recFile = new RECAudioFile(channel, &adc);
  debug("New REC on channel %d\n", channelsUsed);
  RECChannelPos[recChannelsUsed++] = channelsUsed;
  audioChannels[channelsUsed++] = recFile;
}

void AudioPlayer::SDBoot(){  
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
