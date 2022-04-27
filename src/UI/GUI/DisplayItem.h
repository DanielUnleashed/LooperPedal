#ifndef DISPLAYITEM_h
#define DISPLAYITEM_h

#include "Arduino.h"
#include <functional>

#include "UI/Input/DebounceButton.h"

#include <TFT_eSPI.h>

// The FPS whomst value was used to design the animation speeds.
#define FPS_DESIGN 50.0

struct Point{
    uint16_t x, y;
};

class DisplayItem {
    public:
    DisplayItem(String name, uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY);
    DisplayItem(String name); //Full screen item
    ~DisplayItem();
    
    void render(TFT_eSprite&);

    static void startDisplayItems(uint16_t width, uint16_t height);

    //Events
    void addPressEvent(uint8_t pin, std::function<void(void)> func);
    // A released event could be implemented but it may be unnecessary.

    String itemName;
    bool needsUpdate = true; // CURRENTLY needs update.
    void attachRedrawHandler(TaskHandle_t h);
    bool needsToRedraw();
    void forceRedraw();

    private:
    virtual void draw() = 0;

    //Events
    std::function<void(void)> pressFunction;

    Point transformRelativePoint(uint8_t pX, uint8_t pY);

    protected:
    static uint16_t width, height;
    static uint16_t tileSize;
    static uint16_t padX, padY;

    TFT_eSprite *canvas;

    uint8_t sizeX, sizeY;
    uint8_t tileX, tileY;
    uint16_t oX, oY;
    uint8_t inputPin;

    TaskHandle_t redrawHandle;
    void startAnimation();
    void redraw();
    void endAnimation();
    uint32_t startAnimationTime;

    uint32_t getTickTime();

    //General draw functions
    void drawGrid();
    void drawRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color);
    void drawRoundRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color);
    void drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t fillColor);
    void drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t colorA, uint16_t colorB, bool drawHorizontal);
    void drawHLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color);
    void drawVLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color);
    void drawText(uint8_t pX, uint8_t pY, String text, uint16_t color);
    void drawCentreText(uint8_t pX, uint8_t pY, String text, uint16_t color);
    void pushSprite(TFT_eSprite &sprite, uint16_t x, uint16_t y);
};

#endif