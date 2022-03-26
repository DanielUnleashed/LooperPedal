#include "AudioPlayer.h"
#include "utils/AuxSPI.h"

void setup() {
  Serial.begin(115200);
  SPI2.begin();
  
  AudioPlayer::begin();
  AudioPlayer::addAudioFile("/aud1.raw");
  AudioPlayer::addAudioFile("/aud2.raw");
  AudioPlayer::start();
}

void loop() {
 
}
