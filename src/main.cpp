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
  AudioPlayer::addAudioFile("/tereza.wav");
  //AudioPlayer::addAudioFile("/Bass.wav");
  //AudioPlayer::addAudioFile("/Beat.wav");
  //AudioPlayer::addAudioFile("/Beep.wav");
  AudioPlayer::play();
}

void loop() {
  
}

/*#include "chip/adc.h"
#include "chip/dac.h"

#define OUT_FREQ 10000
#define WAIT_TIME 1000000/OUT_FREQ

DAC dac(15);
//ADC adc(26);

void setup(){
  Serial.begin(115200);
  //adc.begin();
  dac.begin();
}


uint32_t t = 0;
void loop(){
  uint16_t val = 0x07FF*(1 + sin(2*PI*440*t/1000000)); 
  dac.write(val);
  Serial.println(val);
  
  delayMicroseconds(WAIT_TIME);
  t+=WAIT_TIME + 30;
}*/