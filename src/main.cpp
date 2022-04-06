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
  AudioPlayer::addAudioFile("/Bass.wav");
  AudioPlayer::addAudioFile("/Beat.wav");
  AudioPlayer::addAudioFile("/Beep.wav");
  AudioPlayer::addAudioFile("/Guitar.wav");
  //AudioPlayer::addAudioFile("/Pad.wav");
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