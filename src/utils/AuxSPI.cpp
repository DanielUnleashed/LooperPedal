#include "AuxSPI.h"

SPIClass AuxSPI::SPI2 = SPIClass(HSPI);
HOLDOUT_PACKET AuxSPI::outputHoldout = {0,0};
SPISettings AuxSPI::spiSettings = SPISettings(20000000, SPI_MSBFIRST, SPI_MODE0);
bool AuxSPI::alreadyDefined = false;
TaskHandle_t AuxSPI::SPI2_Task = NULL;

void AuxSPI::begin(){
    if(alreadyDefined) return;
    SPI2.begin(14,12,13,15);
    xTaskCreatePinnedToCore(SPI2_Sender, "AuxSPISender", 10000, NULL, 7, &SPI2_Task, 0);
    alreadyDefined = true;
}

void AuxSPI::SPI2_Sender(void* funcParams){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(outputHoldout.data != 0){
            write(outputHoldout.data, outputHoldout.pin);
            outputHoldout.data = 0;
        }
    }
    vTaskDelete(NULL);
}

void AuxSPI::writeFromISR(uint8_t chipSelect, uint16_t data){
    outputHoldout = {
        .data = data,
        .pin = chipSelect,
    };
    vTaskNotifyGiveFromISR(SPI2_Task, pdFALSE);
}

void AuxSPI::write(uint8_t chipSelect, uint16_t data){
    digitalWrite(chipSelect, LOW);
    SPI2.beginTransaction(spiSettings);
    SPI2.transfer16(data);
    SPI2.endTransaction();
    digitalWrite(chipSelect, HIGH);
}