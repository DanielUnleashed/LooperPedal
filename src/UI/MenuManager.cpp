#include "MenuManager.h"

TFT_eSPI MenuManager::tft;
uint16_t MenuManager::width, MenuManager::height;

std::vector<Display> MenuManager::displayList;
uint8_t MenuManager::currentDisplay = 0;

DisplayOverlay MenuManager::dispOverlay;

TaskHandle_t MenuManager::drawTaskHandle = NULL;

uint8_t MenuManager::nextDisplay;
bool MenuManager::isInTransition = false;
bool MenuManager::isLaunched = false;

void MenuManager::init(){
#if TOTAL_BUTTONS
    DebounceButton::init();
#endif
#if TOTAL_ROTARY_ENCODERS > 0
    RotaryEncoder::init();
#endif
#if TOTAL_ANALOG_BUTTONS > 0
    //AnalogButton::init();
#endif

    startTFT();

#ifdef USE_DISPLAY
    DisplayItem::startDisplayItems(width, height);
    Widget::startWidgets();

    #ifdef LAUNCH_SPLASHSCREEN_AT_BOOT_UP
        static SplashScreen sp;
        static Display introDisplay("Intro");
        introDisplay.addItem(&sp);
        MenuManager::addDisplay(introDisplay);
    #endif
#else
    tft.fillScreen(0);
    tft.drawCentreString("Screen is off!", width/2, height/2,1);
#endif
}

void MenuManager::launch(){
    #ifdef USE_DISPLAY
        xTaskCreatePinnedToCore(drawTask, "DrawTask", 10000, NULL, 5, &drawTaskHandle, 0);
        dispOverlay.diagonalRadius = sqrt(width*width + height*height);
        displayList[0].launchDisplay();
        isLaunched = true;
    #else
        tft.drawCentreString("Screen is off!", width/2, height/2,1);
    #endif
}

void MenuManager::drawTask(void* funcParams){
    static TFT_eSprite canvas = TFT_eSprite(&tft);
    static SemaphoreHandle_t drawRendered = NULL;
    drawRendered = xSemaphoreCreateMutex();
    xSemaphoreGive(drawRendered);

    canvas.createSprite(width, height);
    canvas.fillSprite(TFT_BLACK);

    for(;;){
        if(isInTransition){
            if(dispOverlay.currentAnimationIndex == 0 || dispOverlay.animationID == DisplayOverlay::ANIM_SWEEP_OUT_LEFT){
                displayList[currentDisplay].forceDraw();
                displayList[currentDisplay].drawDisplay(canvas);
            }
            dispOverlay.render(canvas);

            if(dispOverlay.currentAnimationIndex == dispOverlay.animationQueueSize-1) currentDisplay = nextDisplay;
            
            if(!dispOverlay.needsToRedraw()){ //Ended display overlay animation
                isInTransition = false;
                
                isLaunched = false;
                displayList[nextDisplay].launchDisplay();
                isLaunched = true;
            }
        }else{
            displayList[currentDisplay].drawDisplay(canvas);
        }

        // Pushes the sprite to the screen if the latter frame has been rendered. It is given a 15 ticks margin. If that
        // frame doesn't get rendered, then it skips it. Normally, it will render in 13 ms.
        if(xSemaphoreTake(drawRendered, (TickType_t) 15)) AuxSPI::sendToTFTFromISR(&canvas, drawRendered);
        if(!AudioPlayer::isPlaying) AuxSPI::wakeSPI();

        // Waits for desired minimum FPS
        delay(DRAW_MS);
        // Wait until a DisplayItem needs to update (enhances performance)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

void MenuManager::wakeUpDrawTask(){
    if(drawTaskHandle != NULL){
        xTaskNotifyGive(drawTaskHandle); 
    }else Utilities::error("RedrawHandle is null!\n");
}

void MenuManager::wakeUpDrawTaskFromISR(){
    if(drawTaskHandle != NULL){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(drawTaskHandle, &xHigherPriorityTaskWoken); 
    }else Utilities::error("RedrawHandle is null!\n");
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
    DebounceButton::clearAll();
    RotaryEncoder::clearAll();

    nextDisplay = getDisplayByName(displayName);

#ifdef ENABLE_DISPLAY_ANIMATIONS
    std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, DisplayOverlay::ANIM_SWEEP_OUT_LEFT};
    const std::vector<uint16_t> c{TFT_GOLD, TFT_GOLD};
    dispOverlay.drawMultipleAnimation(v, c);

    isInTransition = true;
#else
    isLaunched = false;
    displayList[nextDisplay].launchDisplay();
    currentDisplay = nextDisplay;
    isLaunched = true;
#endif
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

Display* MenuManager::getCurrentDisplay(){
    return &displayList[currentDisplay];
}

void MenuManager::launchOverlay(uint8_t overlayIndex){
    dispOverlay.drawAnimation(overlayIndex);
}

void MenuManager::startTFT(){
    SPIClass* spi2 = tft.init();
    AuxSPI::begin(spi2);

    tft.setRotation(1);

    width = tft.width();
    height = tft.height();


    drawLoadingMessage("Initializing...");

    Serial.printf("TFT(%dx%d tiles). w=%d  h=%d\n", TILES_X, TILES_Y, width, height);
}

#ifdef ENABLE_DISPLAY_ANIMATIONS
void MenuManager::launchPlayAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                        DisplayOverlay::ANIM_PLAY_TRIANGLE, 
                        DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_GREEN, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();
    isInTransition = true;
}

void MenuManager::launchStopAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                        DisplayOverlay::ANIM_POLYGON | 4, 
                        DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();     
    isInTransition = true;
}

void MenuManager::launchPauseAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                    DisplayOverlay::ANIM_PAUSE, 
                    DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_BLUE, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();  
    isInTransition = true;
}

void MenuManager::launchRecordAnimation(){
    const std::vector<uint8_t> v{DisplayOverlay::ANIM_SWEEP_IN_LEFT, 
                    DisplayOverlay::ANIM_CIRCLE, 
                    DisplayOverlay::ANIM_SWEEP_IN_RIGHT};
    const std::vector<uint16_t> c{TFT_RED, TFT_WHITE, TFT_BLACK};
    dispOverlay.drawMultipleAnimation(v, c);
    displayList[currentDisplay].forceDraw();  
    isInTransition = true;
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
    isInTransition = true;
}

void MenuManager::launchErrorAnimation(String text){
    if(isLaunched){
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

        isInTransition = true;
    }else{
        drawLoadingMessage("- BOOT UP ERROR -", text);
    }
}

void MenuManager::drawLoadingMessage(String line1, String line2, String line3){
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("Stargaze Looper Pedal", width/2, height/6,1);
    int startLine = height/6;

    if(line1!="") tft.drawString(line1, width/2, startLine*3,1);
    if(line2!="") tft.drawString(line2, width/2, startLine*4,1);
    if(line3!="") tft.drawString(line3, width/2, startLine*5,1);
}

#else
    void MenuManager::launchPlayAnimation(){}
    void MenuManager::launchStopAnimation(){}
    void MenuManager::launchPauseAnimation(){}
    void MenuManager::launchRecordAnimation(){}
    void MenuManager::launchWarningAnimation(String text){}
    void MenuManager::launchErrorAnimation(String text){}
#endif