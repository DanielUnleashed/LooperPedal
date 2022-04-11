#ifndef ADC_h
#define ADC_h

#include "Arduino.h"
#include "utils/AuxSPI.h"

#define ADC_CH0 0
#define ADC_CH1 1

class ADC{
    public:
        ADC(uint8_t cs);
        void begin();

        void updateReadings();
        uint16_t getLastReading(bool channel);
        uint16_t readFromISR(bool channel);
        uint16_t read(bool channel);
    private:
        uint8_t readBuffer[2][3] = {{0,0,0},{0,0,0}};
        uint16_t readValue[2] = {0,0};
        uint8_t chipSelect;

};

#endif