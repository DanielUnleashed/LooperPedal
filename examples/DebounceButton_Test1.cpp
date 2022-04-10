#include "UI/DebounceButton.h"

DebounceButton button1(34);

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
}

void loop() {
  if(button1.clicked()){
    digitalWrite(2, HIGH);
    delay(200);
    digitalWrite(2, LOW);
  }
  delay(1);
}