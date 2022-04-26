#ifndef MENUMANAGER_h
#define MENUMANAGER_h

#include "Arduino.h"

#include "GUI/DisplayItem.h"
#include "utils/AuxSPI.h"

#include "UI/Display.h"
#include "UI/GUI/DisplayOverlay.h"
#include "UI/GUI/SplashScreen.h"
#include <list>

#include "defines.h"

#define DRAW_MS 1000/SCREEN_FPS 

class MenuManager {

    public: 
    static void init();
    static void launch();
    static void addDisplay(Display);
    static void removeDisplay(Display);

    static void transitionToDisplay(String dispName, uint8_t transitionAnim);
    static void launchOverlay(uint8_t animationID);
    
    private:
    static TFT_eSPI tft;
    static uint16_t width, height;
    static uint16_t tileW, tileH;

    static std::vector<Display> displayList;
    static Display* currentDisplay;

    static DisplayOverlay dispOverlay;

    static void startTFT();

    static TaskHandle_t drawTaskhandle;
    static void drawTask(void* funcParams);
    static Display getDisplayByName(String name);

    static void launchPlayAnimation();
    static void launchStopAnimation();
    static void launchPauseAnimation();
    static void launchRecordAnimation();
    static void launchWarningAnimation(String text);
    static void launchErrorAnimation(String text);
    
};

#endif