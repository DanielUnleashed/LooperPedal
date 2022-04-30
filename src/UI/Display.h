#ifndef DISPLAY_h
#define DISPLAY_h

#include "Arduino.h"
#include "UI/GUI/DisplayItem.h"
#include "UI/GUI/Widget.h"

class Display {
    public:
    String name;

    Display(String name);
    void addItem(DisplayItem *item);
    void removeItem(DisplayItem *item);

    void drawDisplay(TFT_eSprite &canvas);
    void forceDraw();

    void launchDisplay();

    int8_t hasTaskbar();
    DisplayItem* getDisplayItem(uint8_t index);

    private:
    std::vector<DisplayItem*> itemList;
};

#endif