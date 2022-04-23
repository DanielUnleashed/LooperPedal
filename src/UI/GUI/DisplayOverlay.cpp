#include "DisplayOverlay.h"

DisplayOverlay::DisplayOverlay() : DisplayItem("DisplayOverlay") {}

void DisplayOverlay::draw(){
    if(animationIndex == 0) return;
    else if(animationIndex == 1){
        //FOR TOMORROW!
        
        uint8_t barWidth = 10;
        for(uint8_t t = 0; t < width; t++){
            for(int i = 0; i < height; i+=barWidth*2-1){
                for(int j = 0; j < barWidth; j++){
                    tft->drawFastHLine(t,i+j, j, TFT_YELLOW);
                }
                for(int j = barWidth; j > 0; j--){
                    tft->drawFastHLine(t, i+barWidth*2-j, j, TFT_YELLOW);
                }
            }
        }
    }

}

void DisplayOverlay::drawAnimation(uint8_t animation){
    animationIndex = animation;
}