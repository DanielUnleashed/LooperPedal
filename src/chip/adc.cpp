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

void ADC::getLastReadings(bool channel, uint16_t* buff, uint16_t size){
#ifdef USE_BOTH_ADC_CHANNELS 
    lastReadings[channel].get(buff, size);
#else
    lastReadings.get(buff, size);
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
    uint16_t chA = readFromISR(0);
    // This is so no popping occurs.
    if(chA == 0) chA = 0x8000; // Dire situations need dire solutions...
    lastReadings.put(chA);
    return chA;
#endif
}

uint16_t ADC::readFromISR(bool channel){
    uint8_t data[3] = {0x01u, 0xA0u | (channel << 6), 0x00};
#ifdef USE_BOTH_ADC_CHANNELS
    readValue[channel] = ((readBuffer[channel][1] & 0x0F) << 8) | readBuffer[channel][2];
    AuxSPI::writeAndReadFromISR(chipSelect, data, readBuffer[channel]);
    return readValue[channel];
#else
    readValue = ((readBuffer[1] & 0x0F) << 8) | readBuffer[2];
    AuxSPI::writeAndReadFromISR(chipSelect, data, readBuffer);
    return readValue;
#endif
}

uint16_t ADC::read(bool channel){
    uint8_t data[3] = {0x01u, 0xA0u | (channel << 6), 0x00};
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