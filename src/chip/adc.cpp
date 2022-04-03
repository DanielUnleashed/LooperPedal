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
    uint8_t buff[3] = {0,0,0};
    AuxSPI::writeAndReadFromISR(chipSelect, data, buff);
    Serial.printf("OUT: 0: %02X, 1: %02X, 2: %02X\n", data[0], data[1], data[2]);
    Serial.printf("IN:  0: %02X, 1: %02X, 2: %02X\n", buff[0], buff[1], buff[2]);
    return ((buff[1] & 0x0F) << 8) | buff[2];
}


uint16_t ADC::read(bool channel){
    uint8_t data[3] = {0x01u, 0xA0u | (channel << 6), 0x00};
    uint8_t buff[3] = {0,0,0};
    AuxSPI::writeAndRead(chipSelect, data, buff);
    Serial.printf("OUT: 0: %02X, 1: %02X, 2: %02X\n", data[0], data[1], data[2]);
    Serial.printf("IN:  0: %02X, 1: %02X, 2: %02X\n", buff[0], buff[1], buff[2]);
    return ((buff[1] & 0x0F) << 8) | buff[2];
}