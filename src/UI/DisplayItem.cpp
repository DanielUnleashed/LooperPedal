#include "DisplayItem.h"

DisplayItem::DisplayItem(uint8_t tx, uint8_t ty, uint8_t sx, uint8_t sy){
    tileX = tx;
    tileY = ty;
    sizeX = sx;
    sizeY = sy;
}

DisplayItem::~DisplayItem(){
    
}


void DisplayItem::addPressEvent(uint8_t pin, std::function<void(void)> func){
    pressFunction = func;
//    attachInterrupt(pin, pressEvent, CHANGE);
}
void DisplayItem::addReleaseEvent(uint8_t pin, std::function<void(void)> func){
    releasedFunction = func;
}
void DisplayItem::addLEDEvent(uint8_t LED_index, std::function<void(void)> func){
    LEDFunction = func;
}