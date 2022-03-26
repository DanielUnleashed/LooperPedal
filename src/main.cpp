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

  Utilities::debug("Loading starting files\n");
  AudioPlayer::addAudioFile("/aud1.raw");
  AudioPlayer::addAudioFile("/aud2.raw");

  Utilities::debug("All done!\n");
  AudioPlayer::start();

  //vTaskStartScheduler();
}

void loop() {
 
}
