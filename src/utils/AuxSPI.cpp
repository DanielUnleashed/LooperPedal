#include "AuxSPI.h"

SPIClass* AuxSPI::SPI2 = NULL;
HOLDOUT_PACKET AuxSPI::outputHoldout = {0,0};
bool AuxSPI::alreadyDefined = false;
TaskHandle_t AuxSPI::SPI2_Task = NULL;

void AuxSPI::begin(){
    if(alreadyDefined) return;
    
    SPI2 = new SPIClass(HSPI);
    SPI2 -> begin();
    xTaskCreatePinnedToCore(SPI2_Sender, "AuxSPISender", 10000, NULL, 7, &SPI2_Task, 0);
    alreadyDefined = true;
}

void AuxSPI::SPI2_Sender(void* funcParams){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(outputHoldout.data != 0){
            uint8_t pin = outputHoldout.pin;
            uint16_t data = outputHoldout.data;
            write(pin, data);
        }
    }
    vTaskDelete(NULL);
}

void AuxSPI::writeFromISR(uint8_t chipSelect, uint16_t data){
    outputHoldout.data = data;
    outputHoldout.pin = chipSelect;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(SPI2_Task, &xHigherPriorityTaskWoken);
}

static const SPISettings sett(SPI_CLK, MSBFIRST, SPI_MODE0);
void AuxSPI::write(uint8_t chipSelect, uint16_t data){
    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
    vTaskEnterCritical(&timerMux);
    SPI2 -> beginTransaction(sett);
    digitalWrite(chipSelect, LOW);
    SPI2 -> transfer16(data);
    digitalWrite(chipSelect, HIGH);
    SPI2 -> endTransaction();
    vTaskExitCritical(&timerMux);
}