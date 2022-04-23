#ifndef MENUMANAGER_h
#define MENUMANAGER_h

#include "Arduino.h"

#include "GUI/DisplayItem.h"
#include "utils/AuxSPI.h"

class MenuManager {
    public: 
    static void startMenuManager();
    
    private:
    static TFT_eSPI tft;
    static uint16_t width, height;
    static uint16_t tileW, tileH;

    static void startTFT();
    
};

#endif