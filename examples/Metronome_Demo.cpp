#include "Metronome.h"

Metronome met(4,180,4,4);

void setup() {
  Serial.begin(115200);
  met.start();
}

void loop() {
  met.update();
  delay(100);
}