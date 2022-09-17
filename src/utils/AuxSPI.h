#ifndef AuxSPI_h
#define AuxSPI_h

#include <SPI.h>
#include "Arduino.h"
#include "freertos/task.h"

#include <TFT_eSPI.h>

#define MAX_HOLDOUT_PACKETS 4

/* A HOLDOUT_PACKET will store the data to be sended to the chip when
the ISR ends. It will also wait for a response from the chip if it is 
requested and store the given value in the responseBuffer.
Only one HOLDOUT_PACKET will be stored for each CS, that way there will
be no need to keep controlling the number of requests made. Furthermore, there 
wouldn't be much reason to store different commands to be sent to the same chip
in the same instant. The data released will be the first to be added. */
#pragma pack(push, 1)
struct HOLDOUT_PACKET{
    bool isEmpty;
    uint32_t dataOut;           // Command to send to the chip (maximum 4 bytes)
    uint8_t outLength;          // Number of bytes to send.
    uint8_t pin;                // Chip select (CS)
    uint32_t SPI_speed;
    uint8_t packetType;       // One of the constants below. Indicate the type of the HOLDOUT_PACKET.
    void* responseBuffer;
};
#pragma pack(pop)

class AuxSPI{
    public:
        static const uint8_t RAW_ONLY_READ = 0;
        static const uint8_t RAW_WRITE_READ = 1;
        static const uint8_t LEDS = 2;
        static const uint8_t SCREEN = 3;

        static void begin();
        static void begin(SPIClass* ref);

        static HOLDOUT_PACKET* writeFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data, uint8_t dataLength);
        static HOLDOUT_PACKET* writeFromISR(uint8_t type, uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data, uint8_t dataLength);
        static void write(HOLDOUT_PACKET &packet);

        static HOLDOUT_PACKET* writeAndReadFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* dataOut, uint8_t dataLength, uint8_t* dataInBuff);
        static void writeAndRead(HOLDOUT_PACKET &packet);

        static HOLDOUT_PACKET* sendToLEDsFromISR(uint8_t csPin, uint8_t* data);
        static void sendToLEDs(HOLDOUT_PACKET &packet);
        
        static HOLDOUT_PACKET* sendToTFTFromISR(TFT_eSprite* canvas, SemaphoreHandle_t renderSemaphore);
        static void sendToTFT(HOLDOUT_PACKET &packet);

        static void wakeSPI();

        static void printRealFrequency(uint16_t sampleCount);
        static void chrono(uint16_t sampleCount, uint32_t startTime);
    
    private:
        static SPIClass* SPI2;
        static HOLDOUT_PACKET* holdPackets;
        static bool alreadyDefined;
        static TaskHandle_t SPI2_TaskHandler;

        static void SPI2_Sender(void* funcParams);

        static SemaphoreHandle_t renderedSemaphore;
};

#endif