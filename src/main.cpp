#include "AudioPlayer.h"

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
  AudioPlayer::addSDAudioFile("/tereza.wav");
  AudioPlayer::play();
}

void loop() {
  
}