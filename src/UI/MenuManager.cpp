#include "MenuManager.h"

TFT_eSPI MenuManager::tft;
uint16_t MenuManager::width, MenuManager::height;
uint16_t MenuManager::tileW, MenuManager::tileH;

void MenuManager::startMenuManager(){
    startTFT();
    DisplayItem::startDisplayItems(&tft, width, height, tileW, tileH);
}

void MenuManager::startTFT(){
    SPIClass* spi2 = tft.init();
    AuxSPI::begin(spi2);

    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    width = tft.width();
    height = tft.height();
    tileW = tft.width()/TILES_X;
    tileH = tft.height()/TILES_Y;

    Serial.printf("TFT(%dx%d tiles). w=%d  h=%d\n", TILES_X, TILES_Y, width, height);
}