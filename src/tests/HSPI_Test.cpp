/*#include <SPI.h>
#include "Arduino.h"

SPIClass *hspi = NULL; 
void spiCommand(byte stuff);

void setup(){
  //Serial.begin(115200);
  hspi = new SPIClass(HSPI);
  hspi -> begin();
  pinMode(15, OUTPUT);
}

void loop(){
  spiCommand('e');
  delay(1000);
}

void spiCommand(byte stuff) {
  hspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(15, LOW);
  hspi->transfer(stuff);
  digitalWrite(15, HIGH);
  hspi->endTransaction();
}*/