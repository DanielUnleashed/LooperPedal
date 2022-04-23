#ifndef MENUMANAGER_h
#define MENUMANAGER_h

#include "Arduino.h"

#include "GUI/DisplayItem.h"
#include "utils/AuxSPI.h"

#include "UI/Display.h"
#include <list>

#include "defines.h"

#define DRAW_MS 1000/SCREEN_FPS 

class MenuManager {
    public: 
    static void init();
    static void launch();
    static void addDisplay(Display);
    static void removeDisplay(Display);
    
    private:
    static TFT_eSPI tft;
    static uint16_t width, height;
    static uint16_t tileW, tileH;

    static std::list<Display> displayList;

    static void startTFT();

    static TaskHandle_t drawTaskhandle;
    static void drawTask(void* funcParams);
    
};

#endif