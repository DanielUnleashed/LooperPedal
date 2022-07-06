#include "UI/DebounceButton.h"
#include "defines.h"

DebounceButton button1(PUSH_BUTTON_2);

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  attachInterrupt(PUSH_BUTTON_2, buttonInterrupt, RISING);
}

volatile bool doAnimation = false;
void loop() {
    if(doAnimation){
        digitalWrite(2, HIGH);
        delay(200);
        digitalWrite(2, LOW);
        delay(200);
        doAnimation = false;
    }
}


void IRAM_ATTR buttonInterrupt(){
    if(button1.clicked()) doAnimation = true;
}