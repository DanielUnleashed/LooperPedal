#include "Menu.h"

Menu::Menu(String name) : DisplayItem("Menu"){
    menuName = name;
}

void Menu::draw(){
    //Header
    canvas->fillRoundRect(0,0,width/5, height/4,4, TFT_BLUE);
    canvas->fillRoundRect(width/5,0, width, height/4, 4, TFT_DARKGREY);
    canvas->setTextColor(TFT_BLACK);
    canvas->setTextDatum(ML_DATUM);
    canvas->drawString(menuName, width/5, height/8, 4);

    //List
    const uint8_t itemsPerList = 4;
    static uint16_t oListX = width/5;
    static uint16_t oListY = height/4;
    static uint16_t listItemWidth = 4*width/5;
    static uint16_t listItemHeight = 3*height/4/itemsPerList;
    for(uint8_t i = 0; i < itemsPerList; i++){
        canvas->fillRoundRect(oListX, oListY+i*listItemHeight, listItemWidth, listItemHeight, 4, TFT_GREEN);
    }
}