#include "AudioPlayer.h"

void setup() {
  Serial.begin(115200);
  AudioPlayer::begin();
  AudioPlayer::addAudioFile("/aud1.raw");
  AudioPlayer::addAudioFile("/aud2.raw");
  AudioPlayer::start();
}

void loop() {
 
}
