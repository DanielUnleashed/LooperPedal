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

void ADC::saveLastReadingsToFile(bool channel, File* file, uint16_t size){
#ifdef USE_BOTH_ADC_CHANNELS 
    lastReadings[channel].copyToFile(file, size);
#else
    lastReadings.copyToFile(file, size);
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
    readFromISR(0);
    // This value is the last sampled value, there's a delay of 1/PLAY_FREQUENCY (almost nothing!).
    return readValue;
#endif
}

void ADC::readFromISR(bool channel){
    uint8_t data[3] = {0x01, 0xA0 | (channel << 6), 0x00};
#ifdef USE_BOTH_ADC_CHANNELS
    readValue[channel] = ((readBuffer[channel][1] & 0x0F) << 8) | readBuffer[channel][2];
    AuxSPI::writeAndReadFromISR(chipSelect, data, readBuffer[channel]);
    return readValue[channel];
#else
    // Fetch the last reading from the pointer. If everything works ok, then there should be new data here.
    readValue = ((readBuffer[1] & 0x0F) << 8) | readBuffer[2];
    // Value above is 12 bits long, have to move it to 16 bit so that everything is mixed together ok.
    readValue = readValue << 4;
    // Save the value to be played in the circular buffer.
    lastReadings.testPut(readValue);
    // Sample new data when the ISR ends.
    AuxSPI::writeAndReadFromISR(chipSelect, SPI_Speed, data, 3, readBuffer);
#endif
}