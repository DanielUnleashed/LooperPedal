#include "AudioPlayer.h"
#include "UI/MenuManager.h"

AudioFile* AudioPlayer::audioChannels[MAX_TOTAL_CHANNELS];
uint8_t AudioPlayer::channelsUsed = 0;
hw_timer_t* AudioPlayer::timer;

uint8_t AudioPlayer::SDChannelPos[MAX_AUDIO_CHANNELS];
uint8_t AudioPlayer::RECChannelPos[MAX_REC_CHANNELS];
uint8_t AudioPlayer::audioChannelsUsed = 0;
uint8_t AudioPlayer::recChannelsUsed = 0;

uint8_t AudioPlayer::longestChannel = 0;
CircularBuffer AudioPlayer::globalBuf;

bool AudioPlayer::isPlaying = false;
bool AudioPlayer::isRecording = false;

portMUX_TYPE AudioPlayer::timerMux = portMUX_INITIALIZER_UNLOCKED;
DAC AudioPlayer::dac(CS_DAC);
ADC AudioPlayer::adc(CS_ADC);
Metronome AudioPlayer::metronome;
TaskHandle_t AudioPlayer::audioProcessingTaskHandle;
TaskHandle_t AudioPlayer::statusMonitorTaskHandle;
TaskHandle_t AudioPlayer::memoryTaskHandle;

void AudioPlayer::begin(){
  dac.begin();
  adc.begin();

  delay(10); // Wait for chip initialization

  metronome.start();
  metronome.pause();

  // Input interrupts setup.
  DebounceButton::addInterrupt(4, []{
    if(isPlaying){
      pause();
      MenuManager::launchPauseAnimation();
    }else{
      play();
      MenuManager::launchPlayAnimation();
    }
  });

  DebounceButton::addInterrupt(5, []{  
    for(uint8_t i = 0; i < recChannelsUsed; i++){
      RECAudioFile* currCh = getRECAudioFile(i);
      // This is so that when the stop recording button is pressed, the audio is cut exactly at the end of the beat,
      // which is the same as doing it at the beginning of the beat (beat 0).
      if(isRecording){
        metronome.doAtBeginningOfBeat([currCh](){currCh -> stopRecording();}); 
        MenuManager::launchStopAnimation();
      }else{ // The same is not done while starting a recording as music can start out of the marks of the metronome, for example, an upbeat.
        currCh -> startRecording();
        MenuManager::launchRecordAnimation();
      }
    }
    isRecording = !isRecording;
    if(!isPlaying) play();
  });

  // DebounceButton::addInterrupt(2, []{  
  //   for(uint8_t i = 0; i < recChannelsUsed; i++)
  //     getRECAudioFile(i) -> undoRedoLastRecording();
  // });

#ifdef LAUNCH_CONSOLE
  // Audio processing and signal managing task running on core 0
  xTaskCreatePinnedToCore(statusMonitorTask, "MonitorTask", 10000, NULL, 1, &statusMonitorTaskHandle, 0);
  vTaskSuspend(statusMonitorTaskHandle);
#endif 

  // Memory task running on core 1
  xTaskCreatePinnedToCore(memoryTask, "MemoryTask", 16000, NULL, 5, &memoryTaskHandle, 1);
  vTaskSuspend(memoryTaskHandle);
  
  xTaskCreatePinnedToCore([](void* funcParams){
    for(;;){
      AudioPlayer::metronome.update();
      delay(10);
    }
  }, "MetronomeTask", 4096, NULL, 5, NULL, 1);

  // Start the sampling/playing timer.
  timer = timerBegin(0, 8, true);
  timerAttachInterrupt(timer, frequencyTimer, true);
  timerAlarmWrite(timer, 10000000/PLAY_FREQUENCY, true); //This timer will be called in CORE1.

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
  metronome.resume();
  timerAlarmEnable(timer);
  Serial.println("Play");
}

void AudioPlayer::pause(){
  timerAlarmDisable(timer);
  vTaskSuspend(memoryTaskHandle);
  isPlaying = false;
  setAllTo(SD_FILE_ID, AudioFile::FILE_PAUSED);
  metronome.pause();
  Serial.println("Pause");
}

void AudioPlayer::memoryTask(void* funcParams){ 
  for(;;){
    // Wait for timer notification.
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if(!isPlaying || channelsUsed == 0) continue;

    // Tells all the audio buffers to refresh their contents. If the buffers have already been
    // refreshened then the loop will continue.
    for(uint8_t i = 0; i < channelsUsed; i++) audioChannels[i]->refreshBuffer();

    // Mix all the buffers into the final output.
    uint16_t globalBufLength = globalBuf.getFreeSpace();
    for(uint16_t i = 0; i < globalBufLength; i++){
      uint32_t samples = 0;
      for(uint8_t i = 0; i < channelsUsed; i++){
        samples += audioChannels[i]->getSample();
      }
      uint16_t finalMix = samples/channelsUsed;
      globalBuf.put(finalMix>>4);
    
      if(audioChannels[longestChannel]->hasFileEnded()){
        setAllTo(SD_FILE_ID, AudioFile::FILE_PLAYING);
      }
    }
  }
  vTaskDelete(NULL);
}

void IRAM_ATTR AudioPlayer::frequencyTimer(){
  if(!isPlaying) return;

  portENTER_CRITICAL_ISR(&timerMux);
  #ifdef PASS_AUDIO_INPUT_DURING_RECORDING
    uint16_t adcRead = adc.updateReadings()>>4;
    uint32_t mix;
    mix = (globalBuf.get() + adcRead)>>1;
    dac.writeFromISR(mix);
  #else
    if(isRecording) adc.updateReadings();
    dac.writeFromISR(globalBuf.get());
  #endif

  AuxSPI::wakeSPI();

  if(globalBuf.getFreeSpace() > BUFFER_REFRESH){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(memoryTaskHandle, &xHigherPriorityTaskWoken);
  }
  portEXIT_CRITICAL_ISR(&timerMux);
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

    delay(5000);
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
  debug("File %s added at Channel %d", audioChannels[channelsUsed]->getFileName().c_str(), channelsUsed);
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