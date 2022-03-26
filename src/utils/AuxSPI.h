#ifndef AuxSPI_h
#define AuxSPI_h

#include <SPI.h>

class AuxSPI{
    public:
        void begin();
    private:
        SPIClass SPI2;

};

extern SPIClass SPI2;

#endif