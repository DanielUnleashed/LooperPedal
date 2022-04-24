#ifndef MENUMANAGER_h
#define MENUMANAGER_h

#include "Arduino.h"

#include "GUI/DisplayItem.h"
#include "utils/AuxSPI.h"

#include "UI/Display.h"
#include "UI/GUI/DisplayOverlay.h"
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
    static void launchOverlay(uint8_t animationIndex);
    
    private:
    static TFT_eSPI tft;
    static uint16_t width, height;
    static uint16_t tileW, tileH;

    static std::list<Display> displayList;

    static DisplayOverlay dispOverlay;

    static void startTFT();

    static TaskHandle_t drawTaskhandle;
    static void drawTask(void* funcParams);
    static Display getDisplayByName(String name);
    
};

#endif