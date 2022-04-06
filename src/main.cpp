#include "AudioPlayer.h"
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
  AudioPlayer::addAudioFile("/Rhode.wav");
  AudioPlayer::play();
}

void loop() {
  
}

/*#include "chip/adc.h"

ADC adc(26);

void setup(){
  Serial.begin(115200);
  adc.begin();
}

void loop(){
    Serial.println(adc.read(false));
    delay(500);
}*/