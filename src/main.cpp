#include "UI/DebounceButton.h"

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  DebounceButton::init();
  DebounceButton::addInterrupt(0, []{digitalWrite(2, !digitalRead(2));});
}

void loop() {
}