#include "MenuManager.h"

TFT_eSPI MenuManager::tft;
uint16_t MenuManager::width, MenuManager::height;
uint16_t MenuManager::tileW, MenuManager::tileH;

std::vector<Display> MenuManager::displayList;
uint8_t MenuManager::currentDisplay = 0;

DisplayOverlay MenuManager::dispOverlay;

TaskHandle_t MenuManager::drawTaskhandle = NULL;

void MenuManager::init(){
    startTFT();
    DisplayItem::startDisplayItems(&tft, width, height, tileW, tileH);
    DebounceButton::init();
    RotaryEncoder::init();

#ifdef LAUNCH_SPLASHSCREEN_AT_BOOT_UP
    static SplashScreen sp;
    static Display introDisplay("Intro");
    introDisplay.addItem(&sp);
    MenuManager::addDisplay(introDisplay);
#endif
}

void MenuManager::launch(){
    xTaskCreatePinnedToCore(drawTask, "DrawTask", 10000, NULL, 5, &drawTaskhandle, 0);
    dispOverlay.attachRedrawHandler(drawTaskhandle);
    dispOverlay.diagonalRadius = sqrt(width*width + height*height);

    for(Display d : displayList){
        d.addRedrawHandle(drawTaskhandle);
    }
}

void MenuManager::drawTask(void* funcParams){
    for(;;){
        // Block the display if an overlay is launched.
        if(dispOverlay.needsToRedraw()) dispOverlay.draw();
        else{
            displayList[currentDisplay].drawDisplay();
        }
        delay(DRAW_MS);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

void MenuManager::addDisplay(Display d){
    displayList.push_back(d);
}

void MenuManager::removeDisplay(String displayName){
    for(uint8_t i = 0; i < displayList.size(); i++){
        if(displayList[i].name.equals(displayName));
    }
}

bool MenuManager::changeScreen(String displayName){
    std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_GOLD, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    bool hasBeenFound = false;

    uint8_t clearArray[4];
    for(uint8_t i = 0; i < 4; i++) clearArray[i] = i;
    DebounceButton::clearMultipleInterrupt(clearArray);
    for(uint8_t i = 0; i < displayList.size(); i++){
        if(displayList[i].name.equals(displayName)){
            getDisplayByName(displayName).forceDraw();
            currentDisplay = i;
            hasBeenFound = true;
        }
    }
    if(!hasBeenFound) Utilities::debug("Display %s couldn't be found\n", displayName.c_str());
    return hasBeenFound;
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

void MenuManager::launchPlayAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                        DisplayOverlay::ANIM_PLAY_TRIANGLE, 
                        DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_GREEN, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();   
}

void MenuManager::launchStopAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                        DisplayOverlay::ANIM_POLYGON | 4, 
                        DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();     
}

void MenuManager::launchPauseAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                    DisplayOverlay::ANIM_PAUSE, 
                    DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_BLUE, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();  
}

void MenuManager::launchRecordAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                    DisplayOverlay::ANIM_CIRCLE, 
                    DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();  
}

void MenuManager::launchWarningAnimation(String text){
    std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                        DisplayOverlay::ANIM_CIRCUMFERENCE, 
                        DisplayOverlay::ANIM_EXCLAMATION, 
                        DisplayOverlay::ANIM_TEXT,
                        DisplayOverlay::ANIM_WAIT,
                        DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_ORANGE, TFT_GREENYELLOW, TFT_RED, TFT_BLACK, 0, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    dispOverlay.setAnimationText(text);

    displayList[currentDisplay].forceDraw();  
}

void MenuManager::launchErrorAnimation(String text){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                                DisplayOverlay::ANIM_POLYGON | 6, 
                                DisplayOverlay::ANIM_EXCLAMATION, 
                                DisplayOverlay::ANIM_TEXT,
                                DisplayOverlay::ANIM_WAIT,
                                DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK, TFT_BLACK, 0, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    dispOverlay.setAnimationText(text);

    displayList[currentDisplay].forceDraw();  
}