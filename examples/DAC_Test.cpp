#include "chip/dac.h"

DAC dac(15);
hw_timer_t*   timer;

void IRAM_ATTR frequencyTimer();

void setup(){
  Serial.begin(115200);

  dac.begin();
  timer = timerBegin(0, 8, true);
  timerAttachInterrupt(timer, frequencyTimer, true);
  timerAlarmWrite(timer, 10000000/8000, true);
  timerAlarmEnable(timer);
}

volatile uint16_t t = 0;
void IRAM_ATTR frequencyTimer(){
  dac.writeFromISR(t+=1);
  if(t == 0x0FFF) t = 0;
}

void loop(){
}
