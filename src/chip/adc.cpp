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
#ifdef USE_BOTH_ADC_CHANNELS 
    return readValue[channel];
#else
    return readValue;
#endif
}

CircularBuffer ADC::getLastReadings(bool channel){
#ifdef USE_BOTH_ADC_CHANNELS 
    return lastReadings[channel];
#else
    return lastReadings;
#endif
}

uint16_t ADC::getSavedReadingsCount(bool channel){
#ifdef USE_BOTH_ADC_CHANNELS 
    return lastReadings[channel].getFreeSpace();
#else
    return lastReadings.getWrittenSpace();
#endif
}

uint16_t ADC::updateReadings(){
#ifdef USE_BOTH_ADC_CHANNELS 
    uint16_t chA = readFromISR(0);
    lastReadings[0].put(chA);
    uint16_t chB = readFromISR(1);
    lastReadings[1].put(chB);
    return ((uint32_t)(chA+chB))>>1;
#else
    uint16_t reading = readValue;       
    if(reading == 0) reading = 0x8000;  // This is so no popping occurs. Dire situations need dire solutions...
    lastReadings.put(reading);
    readFromISR(0);
    return reading;
#endif
}

void ADC::readFromISR(bool channel){
    uint8_t data[3] = {0x01, 0xA0 | (channel << 6), 0x00};
#ifdef USE_BOTH_ADC_CHANNELS
    readValue[channel] = ((readBuffer[channel][1] & 0x0F) << 8) | readBuffer[channel][2];
    AuxSPI::writeAndReadFromISR(chipSelect, data, readBuffer[channel]);
    return readValue[channel];
#else
    readValue = ((readBuffer[1] & 0x0F) << 8) | readBuffer[2];
    AuxSPI::writeAndReadFromISR(chipSelect, data, readBuffer);
#endif
}

uint16_t ADC::read(bool channel){
    uint8_t data[3] = {0x01, 0xA0 | (channel << 6), 0x00};
#ifdef USE_BOTH_ADC_CHANNELS
    AuxSPI::writeAndRead(chipSelect, data, readBuffer[channel]);
    readValue[channel] = ((readBuffer[channel][1] & 0x0F) << 8) | readBuffer[channel][2];
    return readValue[channel];
#else
    AuxSPI::writeAndRead(chipSelect, data, readBuffer);
    readValue = ((readBuffer[1] & 0x0F) << 8) | readBuffer[2];
    return readValue;
#endif
}