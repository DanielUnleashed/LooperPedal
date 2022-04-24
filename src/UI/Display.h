#ifndef DISPLAY_h
#define DISPLAY_h

#include "Arduino.h"
#include "UI/GUI/DisplayItem.h"

#include <list>

class Display {
    public:
    String name;

    Display(String name);
    void addItem(DisplayItem *item);
    void removeItem(DisplayItem *item);

    void drawDisplay();
    void forceDraw();
    void addRedrawHandle(TaskHandle_t h);

    private:
    std::list<DisplayItem*> itemList;
    TaskHandle_t redrawHandle;

};

#endif