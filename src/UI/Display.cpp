#include "Display.h"

Display::Display(String n){
    name = n;
}

void Display::drawDisplay(TFT_eSprite &canvas){
    for(DisplayItem* it : itemList)
        if(it -> needsToRedraw()) it -> render(canvas);
}

void Display::forceDraw(){
    for(DisplayItem* it : itemList)
        it -> forceRedraw();
    if(redrawHandle != NULL) xTaskNotifyGive(redrawHandle);
}

void Display::addItem(DisplayItem *item){
    itemList.push_back(item);
    if(redrawHandle != NULL) item -> attachRedrawHandler(redrawHandle);
}

void Display::removeItem(DisplayItem *item){
    for(uint8_t i = 0; i < itemList.size(); i++){
        if(item == itemList[i]){
            itemList.erase(itemList.begin()+i);
            return;
        }
    }
}

void Display::addRedrawHandle(TaskHandle_t h){
    redrawHandle = h;
    for(DisplayItem *it : itemList) it -> attachRedrawHandler(h);
}
