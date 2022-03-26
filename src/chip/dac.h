#ifndef DAC_h
#define DAC_h

#include "Arduino.h"
#include "utils/AuxSPI.h"

class DAC {
    public:
        DAC(uint8_t cs);
        void begin();
        void write(uint16_t data);
        void writeFromISR(uint16_t data);
    private:
        uint8_t chipSelect;
};

#endif