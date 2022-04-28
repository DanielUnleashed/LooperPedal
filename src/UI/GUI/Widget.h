#ifndef WIDGET_h
#define WIDGET_h

#include "DisplayItem.h"

class Widget : public DisplayItem{

    public:
    Widget(String name, uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY);

    static void startWidgets();
    static void increaseCursor();
    static void decreaseCursor();
    static bool isSelectingTile;

    void draw() override;
    virtual void widgetDraw() = 0;

    protected:
    static uint16_t tileSize;
    static uint16_t padX, padY;

    static int8_t cursorPosition;

    bool isWidgetSelected = false;

    uint8_t sizeX, sizeY;
    uint8_t tileX, tileY;
    uint16_t oX, oY;

    Point transformRelativePoint(uint8_t pX, uint8_t pY);

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