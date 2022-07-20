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
        //printRealFrequency(0xFFFF);
        for(uint8_t i = 0; i < holdPacketCount; i++){
            HOLDOUT_PACKET p = holdPackets[i];
            uint8_t* data= (uint8_t*)&p.dataOut;

            if(p.responseType == HOLDOUT_WRITE_READ){
                writeAndRead(p, data);
            }else if(p.responseType == HOLDOUT_ONLY_READ){
                write(p, data);
            }else if(p.responseType == HOLDOUT_LEDS){
                sendToLEDs(p.pin, data);
            }else if(p.responseType == HOLDOUT_SCREEN){
                sendToTFT(p);
                // TODO: No borrar aquello que no se ha mandado. Comprobar que holdPacketCount se resetea o no...
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

HOLDOUT_PACKET* AuxSPI::writeFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data, uint8_t dataLength){
    // Search if a packet already exists, if so return.
    // Data will not be updated, it will send the FIRST data to receive.
    for(uint8_t i = 0; i < holdPacketCount; i++){
        if(holdPackets[i].pin == chipSelect){
            return &holdPackets[i]; //SPI2_Task will already be notified when it gets here.
        }
    }

    // Add a new packet
    holdPackets[holdPacketCount] = {
        .dataOut = 0,
        .outLength = dataLength,
        .pin = chipSelect,
        .SPI_speed = spiSpeed,
        .responseType = HOLDOUT_ONLY_READ,
    };
    for(int i = dataLength-1; i >= 0; i--){
        holdPackets[holdPacketCount].dataOut = (holdPackets[holdPacketCount].dataOut<<8) | data[i];
    }
    return &holdPackets[holdPacketCount++];
}

HOLDOUT_PACKET* AuxSPI::writeAndReadFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* dataOut, uint8_t dataLength, uint8_t* dataInBuff){
    HOLDOUT_PACKET* pack = writeFromISR(chipSelect, spiSpeed, dataOut, dataLength);
    pack -> responseType = HOLDOUT_WRITE_READ;
    pack -> responseBuffer = dataInBuff; // Out buffer will be updated to the latest (more secure?).
    return pack;
}

void AuxSPI::writeAndRead(HOLDOUT_PACKET p, uint8_t* dataOut){
    if(dataOut == NULL) return;
    SPI2 -> beginTransaction(SPISettings(p.SPI_speed, MSBFIRST, SPI_MODE0));
    digitalWrite(p.pin, LOW);
    SPI2 -> transferBytes(dataOut, (uint8_t*)p.responseBuffer, p.outLength);
    digitalWrite(p.pin, HIGH);
    SPI2 -> endTransaction();
}

void AuxSPI::write(HOLDOUT_PACKET p, uint8_t* dataOut){
    if(dataOut == NULL) return;
    SPI2 -> beginTransaction(SPISettings(p.SPI_speed, MSBFIRST, SPI_MODE0));
    digitalWrite(p.pin, LOW);
    SPI2 -> writeBytes(dataOut, p.outLength);
    digitalWrite(p.pin, HIGH);  
    SPI2 -> endTransaction();
}

HOLDOUT_PACKET* AuxSPI::sendToLEDsFromISR(uint8_t chipSelect, uint8_t* dataOut){
    HOLDOUT_PACKET* pack = writeFromISR(chipSelect, 20000000, dataOut, 1);
    pack -> responseType = HOLDOUT_LEDS;
    return pack;
}

void AuxSPI::sendToLEDs(uint8_t csPin, uint8_t* data){
    SPI2 -> beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    SPI2 -> writeBytes(data, 1);
    SPI2 -> endTransaction();
    digitalWrite(csPin, HIGH);
    digitalWrite(csPin, LOW);  
}

HOLDOUT_PACKET* AuxSPI::sendToTFTFromISR(TFT_eSprite* spr){
    holdPackets[holdPacketCount].responseBuffer = spr;
    holdPackets[holdPacketCount].responseType = HOLDOUT_SCREEN;
    return &holdPackets[holdPacketCount++];
}

void AuxSPI::sendToTFT(HOLDOUT_PACKET packet){
    Serial.println("here");
    ((TFT_eSprite*)packet.responseBuffer)->pushSprite(0,0);
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