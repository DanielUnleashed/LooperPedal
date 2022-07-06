#include "AudioPlayer.h"

ADC adc(26);
DAC dac(25);
RECAudioFile* recFile = NULL;
DebounceButton butt1(PUSH_BUTTON_1); 
DebounceButton butt2(PUSH_BUTTON_2); 

TaskHandle_t memoryTaskHandle = NULL;

hw_timer_t* timer;

void recordTask(void* funcParams);
void memoryTask(void* funcParams);

void IRAM_ATTR frequencyTimer();
void startConfig();
void IRAM_ATTR ISR_BUTTON1();
void IRAM_ATTR ISR_BUTTON2();

void setup(){
  startConfig();
  delay(10);

  dac.begin();
  adc.begin();
  recFile = new RECAudioFile(false, &adc);

  xTaskCreatePinnedToCore(memoryTask, "Memorytask", 10000, NULL, 7, &memoryTaskHandle, 1);

  attachInterrupt(PUSH_BUTTON_1, ISR_BUTTON1, CHANGE);
  attachInterrupt(PUSH_BUTTON_2, ISR_BUTTON2, CHANGE);

  timer = timerBegin(0, 8, true);
  timerAttachInterrupt(timer, frequencyTimer, true);
  timerAlarmWrite(timer, 10000000/PLAY_FREQUENCY, true);
  timerAlarmEnable(timer);

  Serial.println("Awaiting input...");
}

void memoryTask(void* funcParams){
  for(;;){
    recFile -> refreshBuffer();
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}

void loop(){}

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR frequencyTimer(){
  portENTER_CRITICAL(&timerMux);
  adc.updateReadings();
  dac.writeFromISR(recFile -> getSample());
  portEXIT_CRITICAL(&timerMux);
}

volatile bool isRecording = false;
void IRAM_ATTR ISR_BUTTON1() {
  if(butt1.clicked()){
    if(isRecording) recFile -> stopRecording();
    else recFile -> startRecording();
    isRecording = !isRecording;
  }
}

void IRAM_ATTR ISR_BUTTON2() {
  if(butt2.clicked()){
    recFile -> undoRedoLastRecording();
  }
}

void startConfig(){
    Serial.begin(115200);
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