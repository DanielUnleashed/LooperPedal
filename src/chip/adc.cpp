#include "chip/adc.h"

ADC::ADC(uint8_t cs){
    chipSelect = cs;
}

void ADC::begin(){
    AuxSPI::begin();
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);
}

uint16_t ADC::readFromISR(bool channel){
    uint8_t data[3] = {0x01u, 0xA0u | (channel << 6), 0x00};
    readValue = ((readBuffer[1] & 0x0F) << 8) | readBuffer[2];
    AuxSPI::writeAndReadFromISR(chipSelect, data, readBuffer);
    return readValue;
}


uint16_t ADC::read(bool channel){
    uint8_t data[3] = {0x01u, 0xA0u | (channel << 6), 0x00};
    AuxSPI::writeAndRead(chipSelect, data, readBuffer);
    readValue = ((readBuffer[1] & 0x0F) << 8) | readBuffer[2];
    return readValue;
}