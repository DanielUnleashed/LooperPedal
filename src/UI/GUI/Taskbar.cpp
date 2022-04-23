#include "Taskbar.h"

Taskbar::Taskbar(uint8_t tileX, uint8_t tileY) : DisplayItem("Taskbar", tileX, tileY, 5, 1){}

void Taskbar::draw(){
    Serial.println("here!");
    /*uint8_t barWidth = 10;
    for(uint8_t t = 0; t < width; t++){
        for(int i = 0; i < height; i+=barWidth*2-1){
            for(int j = 0; j < barWidth; j++){
                tft->drawFastHLine(t,i+j, j, TFT_YELLOW);
            }
            for(int j = barWidth; j > 0; j--){
                tft->drawFastHLine(t, i+barWidth*2-j, j, TFT_YELLOW);
            }
        }
    }*/
    
    drawGrid();
    drawFilledRect(0,0,100,100, 0x190A);
    for(uint8_t i = 0; i < 4; i++){
        if(!buttons[i].isEnabled) continue;

        drawFilledRect(i*25,0, 25,100, 0xEFBE, 0xB63C, true);
        drawCentreText(i*25+13, 50, buttons[i].tagName, TFT_WHITE);
        drawVLine(i*25, 20, 90, TFT_WHITE);
        drawVLine((i+1)*25, 20, 90, TFT_WHITE);
    }

    needsUpdate = false;
}

bool Taskbar::addButton(String tagName, uint8_t index){
    if(index >= TOTAL_BUTTONS) Utilities::error("Button index %d out of bounds (max. %d)\n", index, TOTAL_BUTTONS);
    if(buttons[index].isEnabled) return false;
    
    buttons[index].isEnabled = true;
    buttons[index].tagName = tagName;
    return true;
}

bool Taskbar::removeButton(uint8_t index){
    buttons[index].isEnabled = false;
    return true;
}