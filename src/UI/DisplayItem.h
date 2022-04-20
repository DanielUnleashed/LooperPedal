#ifndef DISPLAYITEM_h
#define DISPLAYITEM_h

#include "Arduino.h"
#include <functional>

#include "DebounceButton.h"

class DisplayItem {

    public:
    DisplayItem(String name, uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY);
    ~DisplayItem();
    virtual void draw() = 0;

    std::function<void(void)> LEDFunction;
    void addLEDTask(uint8_t LED_index, std::function<void(void)> func);
    
    //General draw functions


    //Events
    void addPressEvent(uint8_t pin, std::function<void(void)> func);
    // A released event could be implemented but it may be unnecessary.
    

    private:
    //Events
    std::function<void(void)> pressFunction;

    protected:
    String itemName;
    uint8_t sizeX, sizeY;
    uint8_t tileX, tileY;
    uint8_t inputPin;

};

#endif