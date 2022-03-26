#ifndef DAC_h
#define DAC_h

#include "utils/AuxSPI.h"

class DAC {
    public:
        DAC(uint8_t cs);
        void write(uint16_t data);
    private:
        uint8_t chipSelect;
        SPISettings spiSettings(20000000, SPI_MSBFIRST, SPI_MODE0);
};

#endif