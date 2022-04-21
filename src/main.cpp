#include "AudioPlayer.h"

void setup() {
  Serial.begin(115200);
  Utilities::debug("\n*********************************************\n");
  Utilities::debug("    LOOPER PEDAL v0.1, by DanielUnleashed\n");
  Utilities::debug("*********************************************\n");
  delay(100);

  AudioPlayer::begin();
  AudioPlayer::addRECAudioFile(0);
  AudioPlayer::play();
}

void loop() {}