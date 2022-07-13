#ifndef ADC_h
#define ADC_h

#include "Arduino.h"
#include "utils/AuxSPI.h"
#include "defines.h"
#include "CircularBuffer.h"

#define ADC_CH0 0
#define ADC_CH1 1

class ADC{
    public:
        static const uint32_t SPI_Speed = 1000000;

        ADC(uint8_t cs);
        void begin();

        uint16_t updateReadings();
        uint16_t getLastReading(bool channel = false);
        void readFromISR(bool channel);
        uint16_t read(bool channel);

        CircularBuffer getLastReadings(bool channel);
        uint16_t getSavedReadingsCount(bool channel);

    private:
#ifdef USE_BOTH_ADC_CHANNELS
        uint8_t readBuffer[2][3] = {{0,0,0},{0,0,0}};
        uint16_t readValue[2] = {0,0};
        CircularBuffer lastReadings[2];
#else
        uint8_t readBuffer[3] = {0,0,0};
        uint16_t readValue = 0;
        CircularBuffer lastReadings;
#endif
        uint8_t chipSelect;

};

#endif