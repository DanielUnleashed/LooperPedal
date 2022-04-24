#ifndef DISPLAYITEM_h
#define DISPLAYITEM_h

#include "Arduino.h"
#include <functional>

#include "UI/Input/DebounceButton.h"

#include <TFT_eSPI.h>

#define TILES_X 5
#define TILES_Y 4

struct Point{
    uint16_t x, y;
};

class DisplayItem {

    public:
    DisplayItem(String name, uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY);
    DisplayItem(String name); //Full screen item
    ~DisplayItem();
    virtual void draw() = 0;

    static void startDisplayItems(TFT_eSPI* tft, uint16_t width, uint16_t height, uint16_t tileW, uint16_t tileH);

    std::function<void(void)> LEDFunction;
    void addLEDTask(uint8_t LED_index, std::function<void(void)> func);

    //Events
    void addPressEvent(uint8_t pin, std::function<void(void)> func);
    // A released event could be implemented but it may be unnecessary.

    String itemName;
    bool needsUpdate = true; // CURRENTLY needs update.
    void attachRedrawHandler(TaskHandle_t h);
    bool needsToRedraw();

    private:
    //Events
    std::function<void(void)> pressFunction;

    Point transformRelativePoint(uint8_t pX, uint8_t pY);

    protected:
    static TFT_eSPI *tft;
    static uint16_t width, height;
    static uint16_t tileW, tileH;

    uint8_t sizeX, sizeY;
    uint8_t tileX, tileY;
    uint16_t oX, oY;
    uint8_t inputPin;

    TaskHandle_t redrawHandle;
    void redraw();
    void endAnimation();
    uint32_t startAnimationTime;

    //General draw functions
    void drawGrid();
    void drawRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color);
    void drawRoundRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color);
    void drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t fillColor);
    void drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t colorA, uint8_t colorB, bool drawHorizontal);
    void drawHLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color);
    void drawVLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color);
    void drawText(uint8_t pX, uint8_t pY, String text, uint16_t color);
    void drawCentreText(uint8_t pX, uint8_t pY, String text, uint16_t color);
};

#endif