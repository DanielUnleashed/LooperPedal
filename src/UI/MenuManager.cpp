#include "MenuManager.h"

TFT_eSPI MenuManager::tft;
uint16_t MenuManager::width, MenuManager::height;
uint16_t MenuManager::tileW, MenuManager::tileH;

std::vector<Display> MenuManager::displayList;
Display* MenuManager::currentDisplay;

DisplayOverlay MenuManager::dispOverlay;

TaskHandle_t MenuManager::drawTaskhandle = NULL;

void MenuManager::init(){
    startTFT();
    DisplayItem::startDisplayItems(&tft, width, height, tileW, tileH);
    DebounceButton::init();

    DebounceButton::addInterrupt(0, []{
        //Play animation
        std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, DisplayOverlay::ANIM_SWEEP_IN_OUT | DisplayOverlay::ANIM_POLYGON | 3, DisplayOverlay::ANIM_SWEEP_OUT};
        const std::vector<uint16_t> c{TFT_GREEN, TFT_WHITE, TFT_BLACK};
        dispOverlay.drawMultipleAnimation(v, c);
        getDisplayByName("Main").forceDraw();    
    });

    DebounceButton::addInterrupt(1, []{
        //Stop animation
        std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, DisplayOverlay::ANIM_SWEEP_IN_OUT | DisplayOverlay::ANIM_POLYGON | 4, DisplayOverlay::ANIM_SWEEP_OUT};
        const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
        dispOverlay.drawMultipleAnimation(v, c);
        getDisplayByName("Main").forceDraw();        
    });

    DebounceButton::addInterrupt(2, []{
        transitionToDisplay("Main", DisplayOverlay::ANIM_SWEEP_IN_OUT | DisplayOverlay::ANIM_POLYGON | 5);
    });

    DebounceButton::addInterrupt(3, []{
        transitionToDisplay("Main", DisplayOverlay::ANIM_SWEEP_IN_OUT | DisplayOverlay::ANIM_POLYGON | 6);
    });
}

void MenuManager::launch(){
    xTaskCreatePinnedToCore(drawTask, "DrawTask", 10000, NULL, 5, &drawTaskhandle, 0);
    dispOverlay.attachRedrawHandler(drawTaskhandle);
    dispOverlay.diagonalRadius = sqrt(width*width + height*height);
    currentDisplay = &displayList[0];
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

void MenuManager::transitionToDisplay(String displayName, uint8_t trans){
    Serial.println(trans, HEX);
    if(trans>>7){
        std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, trans, DisplayOverlay::ANIM_SWEEP_OUT};
        const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
        dispOverlay.drawMultipleAnimation(v, c);
        getDisplayByName(displayName).forceDraw();
    }else{
        launchOverlay(trans);
        getDisplayByName(displayName).forceDraw();
    }
}

Display MenuManager::getDisplayByName(String name){
    for(Display d : displayList){
        if(d.name.equals(name)) return d;
    }
    Utilities::error("Could not find display %s\n", name.c_str());
    throw std::runtime_error("MenuManager error!");
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