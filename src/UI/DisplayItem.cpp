#include "DisplayItem.h"

DisplayItem::DisplayItem(String name, uint8_t tx, uint8_t ty, uint8_t sx, uint8_t sy){
    itemName = name;
    tileX = tx;
    tileY = ty;
    sizeX = sx;
    sizeY = sy;
}

DisplayItem::~DisplayItem(){
    DebounceButton::removeInterrupt(inputPin);
}

void DisplayItem::addPressEvent(uint8_t pin, std::function<void(void)> func){
    pressFunction = func;
    inputPin = pin;
    if(!DebounceButton::addInterrupt(inputPin, pressFunction)) Utilities::debug("Failed to add ISR to %s\n", itemName.c_str());
}
void DisplayItem::addLEDTask(uint8_t LED_index, std::function<void(void)> func){
    LEDFunction = func;
}