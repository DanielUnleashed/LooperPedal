#include "Display.h"

Display::Display(String n){
    name = n;
}

void Display::drawDisplay(){
    Serial.println("dip");
    for(DisplayItem* it : itemList){
        if(it -> needsToRedraw()) it -> draw();
    }
}

void Display::addItem(DisplayItem *item){
    itemList.push_back(item);
    if(redrawHandle != NULL) item -> attachRedrawHandler(redrawHandle);
}

void Display::removeItem(DisplayItem *item){
    itemList.remove(item);
}

void Display::addRedrawHandle(TaskHandle_t h){
    redrawHandle = h;
    for(DisplayItem *it : itemList) it -> attachRedrawHandler(h);
}
