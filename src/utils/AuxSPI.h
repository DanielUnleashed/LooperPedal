#ifndef AuxSPI_h
#define AuxSPI_h

#include <SPI.h>
#include "Arduino.h"
#include "freertos/task.h"

#define MAX_HOLDOUT_PACKETS 3 

/* A HOLDOUT_PACKET will store the data to be sended to the chip when
the ISR ends. It will also wait for a response from the chip if it is 
requested and store the given value in the responseBuffer.
Only one HOLDOUT_PACKET will be stored for each CS, that way there will
be no need to keep controlling the number of requests made. Furthermore, there 
wouldn't be much reason to store different commands to be sent to the same chip
in the same instant. The data released will be the last to be added. */
struct HOLDOUT_PACKET{
    uint8_t* dataOut;           // Command to send to the chip.
    uint8_t pin;                // Chip select (CS)
    uint32_t SPI_speed;
    uint8_t responseType;
    uint8_t* responseBuffer;
};

class AuxSPI{
    public:
        static const uint8_t HOLDOUT_ONLY_READ = 0;
        static const uint8_t HOLDOUT_WRITE_READ = 1;
        static const uint8_t HOLDOUT_LEDS = 2;

        static void begin();
        static void begin(SPIClass* ref);
        static HOLDOUT_PACKET* writeFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data);
        static void write(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data);
        static HOLDOUT_PACKET* writeAndReadFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* dataOut, uint8_t* dataInBuff);
        static void writeAndRead(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* dataOut, uint8_t* dataInBuff);
        static HOLDOUT_PACKET* sendToLEDsFromISR(uint8_t csPin, uint8_t* data);
        static void sendToLEDs(uint8_t chipSelect, uint8_t* data);
        

        static void wakeSPI();

        static void printRealFrequency(uint16_t sampleCount);
        static void chrono(uint16_t sampleCount, uint32_t startTime);
    
    private:
        static SPIClass* SPI2;
        static HOLDOUT_PACKET* holdPackets;
        static volatile uint8_t holdPacketCount;
        static bool alreadyDefined;
        static TaskHandle_t SPI2_TaskHandler;

        static void SPI2_Sender(void* funcParams);
};

#endif