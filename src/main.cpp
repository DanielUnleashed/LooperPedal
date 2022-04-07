/*#include "AudioPlayer.h"
#include "utils/AuxSPI.h"
#include "utils/Utilities.h"

char a[] = "/Bass.wav";
char b[] = "/Beat.wav";
char c[] = "/Beep.wav";
char d[] = "/Guitar.wav";
char e[] = "/Pad.wav";

void setup() {
  Serial.begin(115200);
  Utilities::debug("\n*********************************************\n");
  Utilities::debug("    LOOPER PEDAL v0.1, by DanielUnleashed\n");
  Utilities::debug("*********************************************\n");
  delay(1);
  
  AudioPlayer::begin();
  AudioPlayer::addAudioFile(a);
  AudioPlayer::addAudioFile(b);
  AudioPlayer::addAudioFile(c);
  AudioPlayer::addAudioFile(d);
  AudioPlayer::addAudioFile(e);
  //AudioPlayer::addAudioFile("/Rhode.wav");
  AudioPlayer::play();
}

void loop() {
  
}*/

#include "chip/adc.h"
#include "chip/dac.h"
#include "CircularBuffer.h"
#include "SD.h"
#include "FS.h"
#include "AudioPlayer.h"

DAC dac(25);
ADC adc(26);
CircularBuffer buff;

TaskHandle_t recordTaskHandle = NULL;
TaskHandle_t memoryTaskHandle = NULL;

hw_timer_t* timer;

void recordTask(void* funcParams);
void memoryTask(void* funcParams);

void IRAM_ATTR frequencyTimer();

void setup(){
  Serial.begin(115200);
  if (!SD.begin()) Serial.printf("Card Mount Failed\n");
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) Serial.printf("No SD card attached\n");
  
  String cardTypeName = "";
  if (cardType == CARD_MMC) cardTypeName = "MMC";
  else if (cardType == CARD_SD) cardTypeName = "SDSC";
  else if (cardType == CARD_SDHC) cardTypeName = "SDHC";
  else cardTypeName = "UNKNOWN";
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Type: %s. SD Card Size: %lluMB\n", cardTypeName, cardSize);
  
  dac.begin();
  adc.begin();

  xTaskCreatePinnedToCore(recordTask, "RecordTask", 10000, NULL, 7, &recordTaskHandle, 0);
  xTaskCreatePinnedToCore(memoryTask, "Memorytask", 10000, NULL, 7, &memoryTaskHandle, 1);


  timer = timerBegin(0, 8, true);
  timerAttachInterrupt(timer, frequencyTimer, true);
  timerAlarmWrite(timer, 10000000/PLAY_FREQUENCY, true);
  timerAlarmEnable(timer);
}

uint32_t t  = 0;
bool recording = true;
File recordingFile;

void memoryTask(void* funcParams){
  recordingFile = SD.open("/rec.raw", FILE_WRITE);
  
  for(;;){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    //TODO: create circularBuffer.get(size) -> uint8_t*
    uint16_t totalW = buff.getWrittenSpace();
    uint16_t arr[totalW];
    buff.get(arr, totalW);
    recordingFile.write((uint8_t*)arr, totalW*2);
    recordingFile.flush();
  }
}

uint32_t totalIterations = 0;

void recordTask(void* funcParams){
  for(;;){
    totalIterations++;
    if(totalIterations == PLAY_FREQUENCY*10){ // 10 seconds.
      recording = false;
      xTaskNotifyGive(memoryTaskHandle);
    }

    if(recording){
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      buff.put(adc.read(false));

      if(buff.getWrittenSpace() > BUFFER_REFRESH) xTaskNotifyGive(memoryTaskHandle);
    }else{
      delay(3000);

      recordingFile.close();
      vTaskDelete(memoryTaskHandle);
      timerEnd(timer);

      AudioPlayer::begin();
      AudioPlayer::addAudioFile("/rec.raw");
      AudioPlayer::play();

      vTaskDelete(NULL);
    }
  }
}

void loop(){}

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR frequencyTimer(){
  portENTER_CRITICAL(&timerMux);
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(recordTaskHandle, &xHigherPriorityTaskWoken);
  //dac.writeFromISR(buff.get());
  portEXIT_CRITICAL(&timerMux);
}