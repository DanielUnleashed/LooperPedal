#include "AuxSPI.h"

void AuxSPI::begin(){
    SPI2 = SPIClass(HSPI);
    SPI2.begin(14,12,13,15);
}