#include "AudioPlayer.h"
#include "utils/AuxSPI.h"
#include "utils/Utilities.h"

void setup() {
  Serial.begin(115200);
  Utilities::debug("\n*********************************************\n");
  Utilities::debug("    LOOPER PEDAL v0.1, by DanielUnleashed\n");
  Utilities::debug("*********************************************\n");
  delay(1);
  
  AudioPlayer::begin();
  AudioPlayer::addAudioFile("/test_8000.wav");
  
  AudioPlayer::play();

  //vTaskStartScheduler();
}

void loop() {
  
}

