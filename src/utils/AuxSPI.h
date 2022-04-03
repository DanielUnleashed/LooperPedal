#ifndef AuxSPI_h
#define AuxSPI_h

#include <SPI.h>
#include "Arduino.h"
#include "freertos/task.h"

#define SPI_CLK 20000000

/* A HOLDOUT_PACKET will store the data to be sended to the chip when
the ISR ends. It will also wait for a response from the chip if it is 
requested and store the given value in the responseBuffer.
Only one HOLDOUT_PACKET will be stored for each CS, that way there will
be no need to keep controlling the number of requests made. Furthermore, there 
wouldn't be much reason to store different commands to be sent to the same chip
in the same instant.*/
struct HOLDOUT_PACKET{
    uint16_t dataOut;           // Command to send to the chip.
    uint8_t pin;                // Chip select (CS)
    bool needsResponse;
    uint16_t* responseBuffer;
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