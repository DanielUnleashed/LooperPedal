#ifndef AuxSPI_h
#define AuxSPI_h

#include <SPI.h>
#include "Arduino.h"
#include "freertos/task.h"

#define SPI_CLK 20000000

struct HOLDOUT_PACKET{
    uint16_t data;
    uint8_t pin;
};

class AuxSPI{
    public:
        static void begin();
        static void writeFromISR(uint8_t chipSelect, uint16_t data);
        static void write(uint8_t chipSelect, uint16_t data);
    private:
        static SPIClass* SPI2;
        static HOLDOUT_PACKET outputHoldout;
        static bool alreadyDefined;
        static TaskHandle_t SPI2_Task;

        static void SPI2_Sender(void* funcParams);
};

//extern SPIClass SPI2;

#endif