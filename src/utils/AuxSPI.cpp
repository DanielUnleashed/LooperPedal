#include "AuxSPI.h"
#include "AudioPlayer.h"

SPIClass* AuxSPI::SPI2 = NULL;
HOLDOUT_PACKET* AuxSPI::holdPackets = NULL;
bool AuxSPI::alreadyDefined = false;
TaskHandle_t AuxSPI::SPI2_TaskHandler = NULL;

SemaphoreHandle_t AuxSPI::renderedSemaphore = NULL;

void printRealFrequency();

void AuxSPI::begin(){
    if(alreadyDefined) return;
    
    SPI2 = new SPIClass(HSPI);
    SPI2 -> begin();
    holdPackets = (HOLDOUT_PACKET*) calloc(MAX_HOLDOUT_PACKETS, sizeof(HOLDOUT_PACKET));
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
        for(uint8_t i = 0; i < MAX_HOLDOUT_PACKETS; i++){
            // For each type there's an assigned cell in the holdPackets array. If it is 
            // empty, then, continue.
            if(holdPackets[i].isEmpty) continue;
            uint8_t packetType = holdPackets[i].packetType;

            if(packetType == RAW_WRITE_READ){
                writeAndRead(holdPackets[i]);
            }else if(packetType == RAW_ONLY_READ){
                write(holdPackets[i]);
            }else if(packetType == LEDS){
                sendToLEDs(holdPackets[i]);
            }else if(packetType == SCREEN){
                sendToTFT(holdPackets[i]);
            }
        }
        portEXIT_CRITICAL(&timerMux);
    }
    vTaskDelete(NULL);
}

void AuxSPI::wakeSPI(){
    if(xPortInIsrContext()){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(SPI2_TaskHandler, &xHigherPriorityTaskWoken); 
    }else xTaskNotifyGive(SPI2_TaskHandler);
}

HOLDOUT_PACKET* AuxSPI::writeFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data, uint8_t dataLength){
    return writeFromISR(RAW_ONLY_READ, chipSelect, spiSpeed, data, dataLength);
}

HOLDOUT_PACKET* AuxSPI::writeFromISR(uint8_t type, uint8_t chipSelect, uint32_t spiSpeed, uint8_t* data, uint8_t dataLength){
    // Search if a packet already exists, if so return.
    // Data will not be updated, it will send the FIRST data to receive.
    if(!holdPackets[type].isEmpty){
        return &holdPackets[type];
    }

    // Add a new packet
    holdPackets[type] = {
        .isEmpty = false,
        .dataOut = 0,
        .outLength = dataLength,
        .pin = chipSelect,
        .SPI_speed = spiSpeed,
        .packetType = type,
        .responseBuffer = NULL,
    };
    for(int i = dataLength-1; i >= 0; i--){
        holdPackets[type].dataOut = (holdPackets[type].dataOut<<8) | data[i];
    }
    return &holdPackets[type];
}

HOLDOUT_PACKET* AuxSPI::writeAndReadFromISR(uint8_t chipSelect, uint32_t spiSpeed, uint8_t* dataOut, uint8_t dataLength, uint8_t* dataInBuff){
    HOLDOUT_PACKET* pack = writeFromISR(RAW_WRITE_READ, chipSelect, spiSpeed, dataOut, dataLength);
    pack -> responseBuffer = dataInBuff; // Out buffer will be updated to the latest (more secure?).
    return pack;
}

void AuxSPI::writeAndRead(HOLDOUT_PACKET &p){
    SPI2 -> beginTransaction(SPISettings(p.SPI_speed, MSBFIRST, SPI_MODE0));
    digitalWrite(p.pin, LOW);
    SPI2 -> transferBytes((uint8_t*)&p.dataOut, (uint8_t*)p.responseBuffer, p.outLength);
    digitalWrite(p.pin, HIGH);
    SPI2 -> endTransaction();
    p.isEmpty = true;
}

void AuxSPI::write(HOLDOUT_PACKET &p){
    SPI2 -> beginTransaction(SPISettings(p.SPI_speed, MSBFIRST, SPI_MODE0));
    digitalWrite(p.pin, LOW);
    SPI2 -> writeBytes((uint8_t*)&p.dataOut, p.outLength);
    digitalWrite(p.pin, HIGH);  
    SPI2 -> endTransaction();
    p.isEmpty = true;
}

HOLDOUT_PACKET* AuxSPI::sendToLEDsFromISR(uint8_t chipSelect, uint8_t* dataOut){
    HOLDOUT_PACKET* pack = writeFromISR(LEDS, chipSelect, 20000000, dataOut, 1);
    return pack;
}

void AuxSPI::sendToLEDs(HOLDOUT_PACKET &p){
    SPI2 -> beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    SPI2 -> writeBytes((uint8_t*)&p.dataOut, 1);
    SPI2 -> endTransaction();
    digitalWrite(p.pin, HIGH);
    digitalWrite(p.pin, LOW);
    p.isEmpty = true;  
}

HOLDOUT_PACKET* AuxSPI::sendToTFTFromISR(TFT_eSprite* spr, SemaphoreHandle_t renderSemaphore){
    holdPackets[SCREEN].isEmpty = false;
    holdPackets[SCREEN].responseBuffer = spr;
    holdPackets[SCREEN].packetType = SCREEN;
    renderedSemaphore = renderSemaphore;
    return &holdPackets[SCREEN];
}

void AuxSPI::sendToTFT(HOLDOUT_PACKET &p){
    const uint8_t tileSize = 8;
    static uint8_t tileX = 0, tileY = 0;

    if(p.responseBuffer!=NULL){
        TFT_eSprite* spr = (TFT_eSprite*)p.responseBuffer;
        if(AudioPlayer::isPlaying){
            uint8_t maxTileX = spr->width()/tileSize + (spr->width()%tileSize>0);
            uint8_t maxTileY = spr->height()/tileSize + (spr->height()%tileSize>0);

            spr->pushSprite(tileX*tileSize,tileY*tileSize, tileX*tileSize,tileY*tileSize, tileSize, tileSize);
            tileX++;
            tileY += tileX==maxTileX;
            if(tileY == maxTileY){
                if(renderedSemaphore!=NULL) xSemaphoreGive(renderedSemaphore);
                tileX = 0;
                tileY = 0;
                p.isEmpty = true;
            }else{
                tileX %= maxTileX;
            }
        }else{
            spr->pushSprite(0,0);
            p.isEmpty = true;
            if(renderedSemaphore!=NULL) xSemaphoreGive(renderedSemaphore);
        }
    }
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