#include "chip/DAC.h"

DAC::DAC(uint8_t cs){
    chipSelect = cs;
}

void DAC::begin(){
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);
    AuxSPI::begin();
}

void DAC::writeFromISR(uint16_t data){
    // 0x3000 is the header config for the DAC (see datasheet)
    uint16_t out = 0x3000 | (data & 0x0FFF);
    AuxSPI::writeFromISR(chipSelect, out);
}


void DAC::write(uint16_t data){
    // 0x3000 is the header config for the DAC (see datasheet)
    uint16_t out = 0x3000 | (data & 0x0FFF);
    AuxSPI::write(chipSelect, out);
}