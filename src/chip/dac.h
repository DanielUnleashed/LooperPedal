#ifndef DAC_h
#define DAC_h

#include "Arduino.h"
#include "utils/AuxSPI.h"

class DAC {
    public:
        static const uint32_t SPI_Speed = 20000000;

        DAC(uint8_t cs);
        void begin();
        void write(uint16_t data);

        // Sends data to the DAC whilst in ISR. Data must be a 12 bit number.
        void writeFromISR(uint16_t data);
    private:
        uint8_t chipSelect;
};

#endif