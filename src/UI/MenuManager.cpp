#include "MenuManager.h"

TFT_eSPI MenuManager::tft;
uint16_t MenuManager::width, MenuManager::height;

std::vector<Display> MenuManager::displayList;
uint8_t MenuManager::currentDisplay = 0;

DisplayOverlay MenuManager::dispOverlay;

TaskHandle_t MenuManager::drawTaskhandle = NULL;

uint8_t MenuManager::nextDisplay;
bool MenuManager::isInTransition = false;

void MenuManager::init(){
    startTFT();
    DisplayItem::startDisplayItems(width, height);
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
    static TFT_eSprite canvas = TFT_eSprite(&tft);
    canvas.createSprite(width, height);
    canvas.fillSprite(TFT_BLACK);

    for(;;){
        if(isInTransition){
            displayList[currentDisplay].forceDraw();
            displayList[currentDisplay].drawDisplay(canvas);
            dispOverlay.render(canvas);

            if(dispOverlay.animationID == DisplayOverlay::ANIM_SWEEP_OUT_LEFT) currentDisplay = nextDisplay;
            if(!dispOverlay.needsToRedraw()){
                isInTransition = false;
                //Little trick to not modify forceDraw();
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            }
        }else{
            canvas.fillSprite(TFT_BLACK);
            displayList[currentDisplay].drawDisplay(canvas);
        }

        canvas.pushSprite(0, 0);

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
    std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, DisplayOverlay::ANIM_SWEEP_OUT_LEFT};
    const std::vector<uint16_t> c{TFT_GOLD, TFT_GOLD};
    dispOverlay.drawMultipleAnimation(v, c);

    uint8_t clearArray[4];
    for(uint8_t i = 0; i < 4; i++) clearArray[i] = i;
    DebounceButton::clearMultipleInterrupt(clearArray);

    nextDisplay = getDisplayByName(displayName);
    isInTransition = true;
    return true;
}

uint8_t MenuManager::getDisplayByName(String displayName){
    for(uint8_t i = 0; i < displayList.size(); i++){
        if(displayList[i].name.equals(displayName)){
            return i;
        }
    }
    Utilities::error("Could not find display %s\n", displayName.c_str());
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

    Serial.printf("TFT(%dx%d tiles). w=%d  h=%d\n", TILES_X, TILES_Y, width, height);
}

void MenuManager::launchPlayAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                        DisplayOverlay::ANIM_PLAY_TRIANGLE, 
                        DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_GREEN, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();   
}

void MenuManager::launchStopAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                        DisplayOverlay::ANIM_POLYGON | 4, 
                        DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();     
}

void MenuManager::launchPauseAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                    DisplayOverlay::ANIM_PAUSE, 
                    DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_BLUE, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();  
}

void MenuManager::launchRecordAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                    DisplayOverlay::ANIM_CIRCLE, 
                    DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();  
}

void MenuManager::launchWarningAnimation(String text){
    std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                        DisplayOverlay::ANIM_CIRCUMFERENCE, 
                        DisplayOverlay::ANIM_EXCLAMATION, 
                        DisplayOverlay::ANIM_TEXT,
                        DisplayOverlay::ANIM_WAIT,
                        DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_ORANGE, TFT_GREENYELLOW, TFT_RED, TFT_BLACK, 0, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    dispOverlay.setAnimationText(text);

    displayList[currentDisplay].forceDraw();  
}

void MenuManager::launchErrorAnimation(String text){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                                DisplayOverlay::ANIM_POLYGON | 6, 
                                DisplayOverlay::ANIM_EXCLAMATION, 
                                DisplayOverlay::ANIM_TEXT,
                                DisplayOverlay::ANIM_WAIT,
                                DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK, TFT_BLACK, 0, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    dispOverlay.setAnimationText(text);

    displayList[currentDisplay].forceDraw();  
}