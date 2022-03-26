#include "chip/DAC.h"

void DAC::DAC(uint8_t cs){
    chipSelect = cs;
}

void DAC::begin(){
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);
    SPI2.begin();
}

void DAC::write(uint16_t data){
    // 0x3000 is the header config for the DAC (see datasheet)
    uint16_t out = 0x3000 | (data & 0x0FFF);
    digitalWrite(chipSelect, LOW);
    SPI2.beginTransaction();
    SPI2.transfer16(out);
    SPI2.endTransaction();
    digitalWrite(chipSelect, HIGH);

}