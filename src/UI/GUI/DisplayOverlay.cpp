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
    else if(animationID == ANIM_CIRCUMFERENCE){
        double theta = getTickTime()/1000.0*TWO_PI * circumferenceSpeed;
        tft->fillCircle(width/2 + circumferenceRadius*cos(theta + startAngle), height/2 + circumferenceRadius*sin(theta + startAngle),
                         pincelStroke, animationColor);
        animationEnded = theta >= TWO_PI;
    }else if(animationID == ANIM_TRIANGLE){
        double theta = getTickTime()/1000.0*TWO_PI * circumferenceSpeed;
        double n = 5;
        
        double y = TWO_PI/n;
        double theta_rot = theta + rotAngle;
        double mod = y*(theta_rot/y - floor(theta_rot/y));
        double r = cos(PI/n)/cos(mod - PI/n)*circumferenceRadius;
        tft->fillCircle(width/2 + r*cos(theta + startAngle), height/2 + r*sin(theta + startAngle),
                         pincelStroke, animationColor);
        animationEnded = theta >= TWO_PI;
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
        animationQueue.clear();
        animationQueuePalette.clear();
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

void DisplayOverlay::drawMultipleAnimation(std::vector<uint8_t> q, std::vector<uint16_t> colors){
    hasMultipleAnimation = true;
    animationQueue = q;
    animationQueuePalette = colors;
    animationID = animationQueue[0];
    currentAnimationIndex = 0;
    setPalette();
    startAnimation();
}

void DisplayOverlay::setPalette(){
    if(animationQueuePalette.size() > 0) animationColor = animationQueuePalette[currentAnimationIndex];
    else{ //DEFAULT Values
        if(animationID == ANIM_SWEEP_OUT) animationColor = TFT_BLACK;
        else animationColor = TFT_YELLOW;
    } 
}