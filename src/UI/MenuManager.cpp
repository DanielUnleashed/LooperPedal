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
    RotaryEncoder::init();

    DebounceButton::addInterrupt(0, []{
        launchPlayAnimation();
    });

    DebounceButton::addInterrupt(1, []{
        launchStopAnimation();
    });

    DebounceButton::addInterrupt(2, []{
        launchPauseAnimation();
    });

    DebounceButton::addInterrupt(3, []{
        launchWarningAnimation("AAAAAAAH!");
    });

    DebounceButton::addInterrupt(4, []{
        launchErrorAnimation("Shiiit!");
    });

    DebounceButton::addInterrupt(5, []{
        launchWarningAnimation("wowo!");
    });

    RotaryEncoder::addInterrupt(0, [](bool in){
        if(in){
            Serial.println("up!");
        }else{
            Serial.println("down!");
        }
    });

    RotaryEncoder::addButtonInterrupt(0, []{
        Serial.println("foo");
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

void MenuManager::launchPlayAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                        DisplayOverlay::ANIM_PLAY_TRIANGLE, 
                        DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_GREEN, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    currentDisplay -> forceDraw();   
}

void MenuManager::launchStopAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                        DisplayOverlay::ANIM_POLYGON | 4, 
                        DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    currentDisplay -> forceDraw();   
}

void MenuManager::launchPauseAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                    DisplayOverlay::ANIM_PAUSE, 
                    DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_BLUE, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    currentDisplay -> forceDraw();   
}

void MenuManager::launchRecordAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN, 
                    DisplayOverlay::ANIM_CIRCLE, 
                    DisplayOverlay::ANIM_SWEEP_OUT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    currentDisplay -> forceDraw();  
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

    currentDisplay -> forceDraw();   
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

    currentDisplay -> forceDraw();    
}