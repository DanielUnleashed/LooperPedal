#include "AuxSPI.h"

SPIClass* AuxSPI::SPI2 = NULL;
HOLDOUT_PACKET* AuxSPI::holdPackets = NULL;
volatile uint8_t AuxSPI::holdPacketCount = 0;
bool AuxSPI::alreadyDefined = false;
TaskHandle_t AuxSPI::SPI2_TaskHandler = NULL;

void AuxSPI::begin(){
    if(alreadyDefined) return;
    
    SPI2 = new SPIClass(HSPI);
    SPI2 -> begin();
    holdPackets = (HOLDOUT_PACKET*) malloc(MAX_HOLDOUT_PACKETS*sizeof(HOLDOUT_PACKET));
    xTaskCreatePinnedToCore(SPI2_Sender, "AuxSPISender", 10000, NULL, 7, &SPI2_TaskHandler, 0);
    alreadyDefined = true;
}

void AuxSPI::SPI2_Sender(void* funcParams){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
        vTaskEnterCritical(&timerMux);
        for(uint8_t i = 0; i < holdPacketCount; i++){
            if(holdPackets[i].needsResponse){
                writeAndRead(holdPackets[i].pin, holdPackets[i].dataOut, holdPackets[i].responseBuffer);
            }else{
                write(holdPackets[i].pin, holdPackets[i].dataOut);
            }
        }
        holdPacketCount = 0; // Clear all packets.
        vTaskExitCritical(&timerMux);
    }
    vTaskDelete(NULL);
}

HOLDOUT_PACKET* AuxSPI::writeFromISR(uint8_t chipSelect, uint8_t* data){
    // Search if a packet already exists.
    uint8_t i = 0;
    for(i = 0; i < holdPacketCount; i++){
        if(holdPackets[i].pin == chipSelect){
            holdPackets[i].dataOut = data;
            return &holdPackets[i]; //SPI2_Task will already be notified when it gets here.
        }
    }
    // Add a new packet
    holdPackets[holdPacketCount++] = {
        .dataOut = data,
        .pin = chipSelect,
        .needsResponse = false,
    };
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(SPI2_TaskHandler, &xHigherPriorityTaskWoken);
    return &holdPackets[holdPacketCount-1];
}

HOLDOUT_PACKET* AuxSPI::writeAndReadFromISR(uint8_t chipSelect, uint8_t* dataOut, uint8_t* dataInBuff){
    HOLDOUT_PACKET* pack = writeFromISR(chipSelect, dataOut);
    pack -> needsResponse = true;
    pack -> responseBuffer = dataInBuff;
    return pack;
}

static const SPISettings sett(SPI_CLK, MSBFIRST, SPI_MODE0);
void AuxSPI::writeAndRead(uint8_t chipSelect, uint8_t* dataOut, uint8_t* dataInBuff){
    SPI2 -> beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0)); //If substituted by sett, it implodes (doesn't work)
    digitalWrite(chipSelect, LOW);
    SPI2 -> transferBytes(dataOut, dataInBuff, sizeof(dataOut));
    digitalWrite(chipSelect, HIGH);
    SPI2 -> endTransaction();
}

void AuxSPI::write(uint8_t chipSelect, uint8_t* data){
    SPI2 -> beginTransaction(sett);
    digitalWrite(chipSelect, LOW);
    SPI2 -> writeBytes(data, sizeof(data));
    digitalWrite(chipSelect, HIGH);
    SPI2 -> endTransaction();
}