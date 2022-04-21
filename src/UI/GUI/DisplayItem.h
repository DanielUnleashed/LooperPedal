#ifndef DISPLAYITEM_h
#define DISPLAYITEM_h

#include "Arduino.h"
#include <functional>

#include "UI/DebounceButton.h"

#include <TFT_eSPI.h>

#define TILES_X 5
#define TILES_Y 4

class DisplayItem {

    public:
    DisplayItem(String name, uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY);
    ~DisplayItem();
    virtual void draw() = 0;

    static void startTFT();

    std::function<void(void)> LEDFunction;
    void addLEDTask(uint8_t LED_index, std::function<void(void)> func);

    //Events
    void addPressEvent(uint8_t pin, std::function<void(void)> func);
    // A released event could be implemented but it may be unnecessary.

    private:
    //Events
    std::function<void(void)> pressFunction;

    protected:
    static TFT_eSPI tft;
    static uint16_t width, height;
    static uint16_t tileW, tileH;

    String itemName;
    uint8_t sizeX, sizeY;
    uint8_t tileX, tileY;
    uint8_t inputPin;

    //General draw functions
    void drawTiles();

};

#endif