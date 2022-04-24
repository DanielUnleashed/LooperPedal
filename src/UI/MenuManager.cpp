#include "MenuManager.h"

TFT_eSPI MenuManager::tft;
uint16_t MenuManager::width, MenuManager::height;
uint16_t MenuManager::tileW, MenuManager::tileH;

std::list<Display> MenuManager::displayList;

DisplayOverlay MenuManager::dispOverlay;

TaskHandle_t MenuManager::drawTaskhandle = NULL;

void MenuManager::init(){
    startTFT();
    DisplayItem::startDisplayItems(&tft, width, height, tileW, tileH);
}

void MenuManager::launch(){
    xTaskCreatePinnedToCore(drawTask, "DrawTask", 10000, NULL, 5, &drawTaskhandle, 0);
    dispOverlay.attachRedrawHandler(drawTaskhandle);
}

void MenuManager::drawTask(void* funcParams){
    for(;;){
        // Block the display if an overlay is launched.
        if(dispOverlay.needsToRedraw()) dispOverlay.draw();
        else{
            for(Display d : displayList) d.drawDisplay();
        }
        delay(DRAW_MS);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

void MenuManager::addDisplay(Display d){
    displayList.push_back(d);
    d.addRedrawHandle(drawTaskhandle);
}

void MenuManager::removeDisplay(Display d){
    //displayList.remove(d); <- Gives error whoops!
    // Maybe I shall define a == statement, but... neh!
}

void MenuManager::launchOverlay(uint8_t overlayIndex){
    dispOverlay.drawAnimation(overlayIndex);
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