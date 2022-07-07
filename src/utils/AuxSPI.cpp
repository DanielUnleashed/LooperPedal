#include "AuxSPI.h"

SPIClass* AuxSPI::SPI2 = NULL;
HOLDOUT_PACKET* AuxSPI::holdPackets = NULL;
volatile uint8_t AuxSPI::holdPacketCount = 0;
bool AuxSPI::alreadyDefined = false;
TaskHandle_t AuxSPI::SPI2_TaskHandler = NULL;

void printRealFrequency();

void AuxSPI::begin(){
    if(alreadyDefined) return;
    
    SPI2 = new SPIClass(HSPI);
    SPI2 -> begin();
    holdPackets = (HOLDOUT_PACKET*) malloc(MAX_HOLDOUT_PACKETS*sizeof(HOLDOUT_PACKET));
    xTaskCreatePinnedToCore(SPI2_Sender, "AuxSPISender", 10000, NULL, 10, &SPI2_TaskHandler, 0);
    alreadyDefined = true;
}

/* This method is used with a passed reference. In this case, if the tft is used, it will instantiate
*  a SPIClass attached to VSPI which cannot be instantiated twice. */
void AuxSPI::begin(SPIClass* spiref){
    if(alreadyDefined) return;
    SPI2 = spiref;
    holdPackets = (HOLDOUT_PACKET*) malloc(MAX_HOLDOUT_PACKETS*sizeof(HOLDOUT_PACKET));
    xTaskCreatePinnedToCore(SPI2_Sender, "AuxSPISender", 10000, NULL, 10, &SPI2_TaskHandler, 0);
    alreadyDefined = true;
}

void AuxSPI::SPI2_Sender(void* funcParams){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
        portENTER_CRITICAL(&timerMux);
        printRealFrequency(0xFFFF);
        for(uint8_t i = 0; i < holdPacketCount; i++){
            if(holdPackets[i].responseType == HOLDOUT_WRITE_READ){
                writeAndRead(holdPackets[i].pin, holdPackets[i].SPI_speed, holdPackets[i].dataOut, holdPackets[i].responseBuffer);
            }else if(holdPackets[i].responseType == HOLDOUT_ONLY_READ){
                write(holdPackets[i].pin, holdPackets[i].SPI_speed, holdPackets[i].dataOut);
            }else if(holdPackets[i].responseType == HOLDOUT_LEDS){
                sendToLEDs(holdPackets[i].pin, holdPackets[i].dataOut);
            }
        }
        holdPacketCount = 0; // Clear all packets.
        portEXIT_CRITICAL(&timerMux);
    }
    vTaskDelete(NULL);
}

void AuxSPI::wakeSPI(){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(SPI2_TaskHandler, &xHigherPriorityTaskWoken); 
}

HOLDOUT_PACKET* AuxSPI::writeFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data){
    // Search if a packet already exists.
    for(uint8_t i = 0; i < holdPacketCount; i++){
        if(holdPackets[i].pin == chipSelect){
            holdPackets[i].dataOut = data;
            return &holdPackets[i]; //SPI2_Task will already be notified when it gets here.
        }
    }

    // Add a new packet
    holdPackets[holdPacketCount++] = {
        .dataOut = data,
        .pin = chipSelect,
        .SPI_speed = spiSpeed,
        .responseType = HOLDOUT_ONLY_READ,
    };
    wakeSPI();
    return &holdPackets[holdPacketCount-1];
}

HOLDOUT_PACKET* AuxSPI::writeAndReadFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* dataOut, uint8_t* dataInBuff){
    HOLDOUT_PACKET* pack = writeFromISR(chipSelect, spiSpeed, dataOut);
    pack -> responseType = HOLDOUT_WRITE_READ;
    pack -> responseBuffer = dataInBuff;
    return pack;
}

void AuxSPI::writeAndRead(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* dataOut, uint8_t* dataInBuff){
    if(dataOut == NULL) return;
    SPI2 -> beginTransaction(SPISettings(spiSpeed, MSBFIRST, SPI_MODE0));
    digitalWrite(chipSelect, LOW);
    SPI2 -> transferBytes(dataOut, dataInBuff, sizeof(dataOut));
    digitalWrite(chipSelect, HIGH);
    SPI2 -> endTransaction();
}

void AuxSPI::write(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data){
    if(data == NULL) return;
    SPI2 -> beginTransaction(SPISettings(spiSpeed, MSBFIRST, SPI_MODE0));
    digitalWrite(chipSelect, LOW);
    SPI2 -> writeBytes(data, sizeof(data));
    digitalWrite(chipSelect, HIGH);  
    SPI2 -> endTransaction();
}

HOLDOUT_PACKET* AuxSPI::sendToLEDsFromISR(uint8_t chipSelect, uint8_t* dataOut){
    HOLDOUT_PACKET* pack = writeFromISR(chipSelect, 5000, dataOut);
    pack -> responseType = HOLDOUT_LEDS;
    return pack;
}

void AuxSPI::sendToLEDs(uint8_t csPin, uint8_t* data){
    SPI2 -> beginTransaction(SPISettings(5000, MSBFIRST, SPI_MODE0));
    SPI2 -> writeBytes(data, 1);
    SPI2 -> endTransaction();
    digitalWrite(csPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(csPin, LOW);  
}

void AuxSPI::printRealFrequency(uint16_t sampleCount){
    static uint32_t lastCall = micros();
    static uint32_t average = 0;
    static uint16_t it = 0;
    static uint32_t min = 0xFFFF;
    static uint32_t max = 0;

    uint32_t now = micros();
    uint32_t real = now-lastCall;
    if(real < min) min = real;
    else if(real > max) max = real;
    average += real;
    it++;
    if(it == sampleCount){
        Serial.printf("\nfreq=%d Hz, min=%d max=%d us\n", 1000000/(average/sampleCount), min, max);
        it = 0;
        average = 0;
        min = 0xFFFF;
        max = 0;
    }
    lastCall = now;
}

void AuxSPI::chrono(uint16_t sampleCount, uint32_t startTime){
    static uint32_t average = 0;
    static uint16_t it = 0;
    static uint32_t min = 0xFFFF;
    static uint32_t max = 0;
    
    uint32_t real = micros()-startTime;
    if(real < min) min = real;
    else if(real > max) max = real;
    average += real;
    it++;
    if(it == sampleCount){
        Serial.printf("\naver=%d us, min=%d max=%d us\n", average/sampleCount, min, max);
        it = 0;
        average = 0;
        min = 0xFFFF;
        max = 0;
    }
}