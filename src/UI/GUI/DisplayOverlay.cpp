#include "DisplayOverlay.h"

DisplayOverlay::DisplayOverlay() : DisplayItem("DisplayOverlay") {
    needsUpdate = false;
}

void DisplayOverlay::draw(){
    bool animationEnded = false;
    if(animationIndex == ANIM_NONE) endAnimation();
    else if(animationIndex == ANIM_SWEEP){
        uint32_t t = getTickTime();
        t = t*t*t/10000; //Ease in, faster out
        t *= sweepSpeed; 
        for(int i = 0; i < height; i+=barWidth*2-1){
            for(int j = 0; j < barWidth; j++){
                tft->drawFastHLine(t,i+j, j, TFT_YELLOW);
            }
            for(int j = barWidth; j > 0; j--){
                tft->drawFastHLine(t, i+barWidth*2-j, j, TFT_YELLOW);
            }
            tft->fillRect(t-15,0, 15,height, TFT_YELLOW);
        }
        animationEnded = t>width;
    }else if(animationIndex == ANIM_CIRCLE){
        uint32_t t = getTickTime();
        t = t*t*t/10000; //Ease in, faster out
        t *= circleSpeed; 
        tft->fillCircle(width/2, height/2, t, TFT_YELLOW);
        animationEnded = t > diagonalRadius;
    }

    if(animationEnded){
        endAnimation();
        tft->fillScreen(TFT_BLACK);
        if(redrawHandle != NULL) xTaskNotifyGive(redrawHandle);
    }else redraw();
}

void DisplayOverlay::drawAnimation(uint8_t animation){
    animationIndex = animation;
    startAnimation();
}