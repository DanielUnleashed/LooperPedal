#ifndef WIDGET_h
#define WIDGET_h

#include "DisplayItem.h"

class Widget : public DisplayItem{

    public:
    static const uint8_t NONE_EVENT = 0;
    static const uint8_t INCREASE_CURSOR = 1;
    static const uint8_t DECREASE_CURSOR = 2;
    static const uint8_t SWITCH_SELECTION_MODES = 3;
    static const uint8_t UNDO_WIDGET_SELECTION = 4;
    static const uint8_t DELETE_SELECTED_WIDGET = 5;

    Widget(String name, uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY, 
        uint8_t inWidgetSelectables);

    String widgetName;
    uint16_t widgetID;
    bool hasBeenPlaced = true;

    uint8_t inWidgetSelectables;

    static void startWidgets();
    static void increaseCursor();
    static void decreaseCursor();
    static void undoWidgetSelection();
    static void deleteSelectedWidget();

    static bool isWidgetSelectionMode;

    static void startDraw(TFT_eSprite &canvas);
    virtual void widgetDraw() = 0;
    static void finalDraw(TFT_eSprite &canvas);

    static bool tileMapOverlap[TILES_X][TILES_Y];

    static void widgetEventTask(void* funcParams);
    static TaskHandle_t widgetEventHandle;
    static uint8_t widgetEvent;

    static std::vector<Widget*> displayedWidgets;
    static void addWidget(Widget* w);
    static void removeWidget(Widget* w);
    static void clearWidgets();
    // Will sort the list above according to placement priorities.
    static void sortDisplayedWidgetsList();
    static bool areTilesOverlapping();
    static bool getTileOverlapMap();
    static void redrawAll();

    static void switchSelectionMode();

    protected:
    static uint16_t tileSize;
    static uint16_t padX, padY;

    // For holding a widget
    static int8_t holdingPosition;
    static int8_t previousHoldingPosition;
    // For moving through the interface
    static int8_t selectedWidget;
    static int8_t inWidgetSelection;

    bool isSelected();

    uint8_t sizeX, sizeY;
    uint8_t tileX, tileY;
    uint16_t oX, oY;

    Point transformRelativePoint(uint8_t pX, uint8_t pY);
    void recalculateRelativePoint();

    //General draw functions
    static void drawGrid(TFT_eSprite &canvas, uint16_t color);
    void drawRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color);
    void drawRoundRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color);
    void drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t fillColor);
    void drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t colorA, uint16_t colorB, bool drawHorizontal);
    void drawHLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color);
    void drawVLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color);
    void drawText(uint8_t pX, uint8_t pY, String text, uint16_t color);
    void drawCentreText(uint8_t pX, uint8_t pY, String text, uint16_t color);
    void pushSprite(TFT_eSprite &sprite, uint16_t x, uint16_t y);

    static void fillTile(TFT_eSprite &canvas, uint8_t x, uint8_t y, uint16_t color);

    private:
    void draw() override;

};

#endif