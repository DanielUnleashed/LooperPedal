#include "chip/DAC.h"

DAC::DAC(uint8_t cs){
    chipSelect = cs;
}

void DAC::begin(){
    AuxSPI::begin();
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);
}

void DAC::writeFromISR(uint16_t data){
    // 0x3000 is the header config for the DAC (see datasheet)
    uint16_t out = 0x3000 | (data & 0x0FFF);
    uint8_t buf[2] = {out>>8, out};
    AuxSPI::writeFromISR(chipSelect, SPI_Speed, buf);
}


void DAC::write(uint16_t data){
    // 0x3000 is the header config for the DAC (see datasheet)
    uint16_t out = 0x3000 | (data & 0x0FFF);
    uint8_t buf[2] = {out>>8, out};
    AuxSPI::write(chipSelect, SPI_Speed, buf);
}