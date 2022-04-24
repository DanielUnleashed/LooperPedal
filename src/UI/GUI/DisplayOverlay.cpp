#include "DisplayOverlay.h"

DisplayOverlay::DisplayOverlay() : DisplayItem("DisplayOverlay") {
    needsUpdate = false;
}

void DisplayOverlay::draw(){
    if(animationIndex == ANIM_NONE) endAnimation();
    else if(animationIndex == ANIM_SWEEP){
        uint32_t t = sweepSpeed*(millis() - startAnimationTime)/1000;
        for(int i = 0; i < height; i+=barWidth*2-1){
            for(int j = 0; j < barWidth; j++){
                tft->drawFastHLine(t,i+j, j, TFT_YELLOW);
            }
            for(int j = barWidth; j > 0; j--){
                tft->drawFastHLine(t, i+barWidth*2-j, j, TFT_YELLOW);
            }
            tft->fillRect(t-4,0, 4, height, TFT_YELLOW);
        }
        if(t < width) redraw();
        else endAnimation();
    }

}

void DisplayOverlay::drawAnimation(uint8_t animation){
    Serial.println("anim");
    animationIndex = animation;
    startAnimationTime = millis();
    redraw();
}