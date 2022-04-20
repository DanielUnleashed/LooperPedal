#ifndef DISPLAYITEM_h
#define DISPLAYITEM_h

#include "Arduino.h"
#include <functional>

class DisplayItem {

    public:
    DisplayItem(uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY);
    ~DisplayItem();
    virtual void draw() = 0;

    //Draw functions

    //Events
    void addPressEvent(uint8_t pin, std::function<void(void)> func);
    void addReleaseEvent(uint8_t pin, std::function<void(void)> func);
    void addLEDEvent(uint8_t LED_index, std::function<void(void)> func);

    private:
    //Events
    std::function<void(void)> pressFunction = NULL;
    std::function<void(void)> releasedFunction = NULL;
    std::function<void(void)> LEDFunction = NULL;

    static void IRAM_ATTR pressEvent1();

    protected:
    uint8_t sizeX, sizeY;
    uint8_t tileX, tileY;

};

#endif