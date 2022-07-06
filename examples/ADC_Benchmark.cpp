#include "chip/adc.h"

ADC adc(26);

void setup(){
  Serial.begin(115200);
  adc.begin();

  uint32_t el = 0;
  for(uint8_t i = 0; i < 10; i++){
    uint32_t start = micros();
    uint16_t d = adc.read(0);
    el += micros() - start;
    Serial.printf("%d  =  0x%04X\n", d, d);
  }

  el /= 10;
  Serial.printf("el: %d\n", el);  //47 us -> 20 kHz

}

void loop(){

}