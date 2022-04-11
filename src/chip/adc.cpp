#include "chip/adc.h"

ADC::ADC(uint8_t cs){
    chipSelect = cs;
}

void ADC::begin(){
    AuxSPI::begin();
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);
}

uint16_t ADC::getLastReading(bool channel){
    return readValue[channel];
}

void ADC::updateReadings(){
    readFromISR(0);
    //readFromISR(1);
}

uint16_t ADC::readFromISR(bool channel){
    uint8_t data[3] = {0x01u, 0xA0u | (channel << 6), 0x00};
    readValue[channel] = ((readBuffer[channel][1] & 0x0F) << 8) | readBuffer[channel][2];
    AuxSPI::writeAndReadFromISR(chipSelect, data, readBuffer[channel]);
    return readValue[channel];
}

uint16_t ADC::read(bool channel){
    uint8_t data[3] = {0x01u, 0xA0u | (channel << 6), 0x00};
    AuxSPI::writeAndRead(chipSelect, data, readBuffer[channel]);
    readValue[channel] = ((readBuffer[channel][1] & 0x0F) << 8) | readBuffer[channel][2];
    return readValue[channel];
}