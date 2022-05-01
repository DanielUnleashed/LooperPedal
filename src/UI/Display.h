#ifndef DISPLAY_h
#define DISPLAY_h

#include "Arduino.h"
#include "UI/GUI/DisplayItem.h"
#include "UI/GUI/Widget.h"

#include "UI/GUI/Taskbar.h"

class Display {
    public:
    String name;

    Display(String name);
    void addItem(DisplayItem *item);
    void removeItem(DisplayItem *item);

    void drawDisplay(TFT_eSprite &canvas);
    void forceDraw();

    void launchDisplay();

    void addTaskbar();

    bool hasTaskbar();
    Taskbar* getTaskbar();
    DisplayItem* getDisplayItem(uint8_t index);

    private:
    std::vector<DisplayItem*> itemList;
    Taskbar* taskbar = NULL;
};

#endif