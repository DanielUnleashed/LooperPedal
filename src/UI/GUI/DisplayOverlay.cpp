#include "DisplayOverlay.h"

DisplayOverlay::DisplayOverlay() : DisplayItem("DisplayOverlay") {
    needsUpdate = false;
}

void DisplayOverlay::draw(){
    bool animationEnded = false;
    if(animationID == ANIM_NONE) endAnimation();
    else if(animationID == ANIM_SWEEP_IN){
        long t = getTickTime();
        t = t*t*t/10000; //Ease in, faster out
        t *= sweepSpeed;
        t -= barWidth;  // So that the animation start from  
        for(int i = 0; i < height; i+=barWidth*2-1){
            for(int j = 0; j < barWidth; j++){
                tft->drawFastHLine(t,i+j, j, animationColor);
            }
            for(int j = barWidth; j > 0; j--){
                tft->drawFastHLine(t, i+barWidth*2-j, j, animationColor);
            }
            tft->fillRect(t-15,0, 15,height, animationColor);
        }
        animationEnded = t>width;
    }else if(animationID == ANIM_SWEEP_OUT){
        tft->setRotation(3); // This is what I like to call a little trickery!
        animationID = ANIM_SWEEP_IN;
        draw();
    }else if(animationID == ANIM_CIRCLE){
        uint32_t t = getTickTime();
        t = t*t*t/10000; //Ease in, faster out
        t *= circleSpeed; 
        tft->fillCircle(width/2, height/2, t, animationColor);
        animationEnded = t > diagonalRadius;
    }

    if(animationEnded){
        if(hasMultipleAnimation){
            currentAnimationIndex++;
            if(currentAnimationIndex != animationQueue.size()){
                animationID = animationQueue[currentAnimationIndex];
                Serial.println(animationID);
                setPalette();
                startAnimation();
                return;
            }
        }
        endAnimation();
        tft->setRotation(1);
        if(redrawHandle != NULL) xTaskNotifyGive(redrawHandle);
    }else redraw();
}

void DisplayOverlay::drawAnimation(uint8_t animation){
    hasMultipleAnimation = false;
    animationID = animation;
    setPalette();
    startAnimation();
}

void DisplayOverlay::drawMultipleAnimation(std::vector<uint8_t> q){
    hasMultipleAnimation = true;
    animationQueue = q;
    animationID = animationQueue[0];
    currentAnimationIndex = 0;
    setPalette();
    startAnimation();
}

void DisplayOverlay::setPalette(){
    if(animationID == ANIM_SWEEP_OUT) animationColor = TFT_BLACK;
    else animationColor = TFT_YELLOW; 
}