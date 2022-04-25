#include "DisplayOverlay.h"

DisplayOverlay::DisplayOverlay() : DisplayItem("DisplayOverlay") {
    needsUpdate = false;
}

void DisplayOverlay::draw(){
    bool animationEnded = false;
    if(animationID == ANIM_WAIT){
        long t = getTickTime();
        animationEnded = t > waitTime;
    }else if(animationID == ANIM_SWEEP_IN){
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
    }else if(animationID == ANIM_CIRCUMFERENCE){
        double theta = getTickTime()/1000.0*TWO_PI * plottingSpeed;
        tft->fillCircle(width/2 + plottingRadius*cos(theta + HALF_PI), height/2 + plottingRadius*sin(theta + HALF_PI),
                         outerPincelStroke, animationColor);
        double r = plottingRadius*theta;
        tft->fillCircle(width/2 + r*cos(theta + HALF_PI), height/2 + r*sin(theta + HALF_PI),
                         innerPincelStroke, animationColor);
        animationEnded = theta >= TWO_PI;
    }else if(animationID == ANIM_EXCLAMATION){
        double theta = getTickTime()/1000.0*TWO_PI;
        theta += 7.0*PI/16.0;
        double r = -7.0*plottingRadius*cos(8.0*theta)/8.0;
        double c1 = height/2 + 7.0*plottingRadius/32.0;
        //This is part of a rose of 8 pethals. Is the bar of the exclamation.
        tft->fillCircle(width/2 + r*cos(theta), c1 + r*sin(theta),
                            innerPincelStroke, animationColor);
        animationEnded = theta >= 9.0*PI/16.0;
        if(animationEnded){
            //Exclamation dot
            tft -> fillCircle(width/2, height/2 + 4.0*plottingRadius/7.0,
                            outerPincelStroke, animationColor);
        }
    }else if(animationID == ANIM_PLAY_TRIANGLE){
        animationEnded = drawNGon(3, 0);
    }else if(animationID == ANIM_PAUSE){
        double theta = getTickTime()/1000.0*TWO_PI* plottingSpeed;
        double k = 10, n = 4, a = 0.5, b = 2;
        double r = k*a*b/pow(pow(b*cos(theta), n) + pow(a*sin(theta), n), 1/n);
        tft->fillCircle(7.0*width/18.0 + r*cos(theta), height/2 + r*sin(theta),
                    outerPincelStroke, animationColor);
        tft->fillCircle(11.0*width/18.0 + r*cos(theta), height/2 + r*sin(theta),
                    outerPincelStroke, animationColor);
        animationEnded = theta >= TWO_PI;
    }else if((animationID&0x40) == ANIM_POLYGON){
        uint8_t shape = animationID&0x0F;
        double rotAngle = 0;
        if(shape == 3) rotAngle = -0.524;
        if(shape == 4) rotAngle = 0.785;
        if(shape == 5) rotAngle = 0.315;
        if(shape == 7) rotAngle = -0.224; 
        animationEnded = drawNGon(shape, rotAngle);
    }else if(animationID == ANIM_TEXT){
        tft->setTextColor(animationColor);
        tft->setTextDatum(MC_DATUM);
        tft->drawString(animationText, width/2, 0.5*(plottingRadius+3.0*height/2.0));
        animationEnded = true;
    }

    if(animationEnded){
        if(hasMultipleAnimation){
            currentAnimationIndex++;
            if(currentAnimationIndex != animationQueue.size()){
                animationID = animationQueue[currentAnimationIndex];
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
// Idea "leased" from https://www.geogebra.org/m/cXXGKUQk
// Since TFT screen's coords are Y flipped, take that in mind during the dessign!
bool DisplayOverlay::drawNGon(uint8_t sides, double rotAngle){
    double theta = getTickTime()/1000.0*TWO_PI * plottingSpeed;
    double y = TWO_PI/sides;
    double theta_rot = theta + rotAngle;

    if(theta < TWO_PI){
        double mod = y*(theta_rot/y - floor(theta_rot/y));
        double r = cos(PI/sides)/cos(mod - PI/sides)*plottingRadius;
        tft->fillCircle(width/2 + r*cos(theta), height/2 + r*sin(theta),
                        outerPincelStroke, animationColor);
    }
    double mod = y*(theta_rot/y - floor(theta_rot/y));
    double r = cos(PI/sides)/cos(mod - PI/sides)*plottingRadius * theta_rot / (4.0*PI);
    tft->fillCircle(width/2 + r*cos(theta), height/2 + r*sin(theta),
                        innerPincelStroke, animationColor);

    return theta >= 3.75*PI;
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

void DisplayOverlay::setAnimationText(String str){
    animationText = str;
}

void DisplayOverlay::setPalette(){
    if(animationQueuePalette.size() > 0){
        if(currentAnimationIndex >= animationQueuePalette.size()) animationColor = TFT_PINK;
        else animationColor = animationQueuePalette[currentAnimationIndex];
    }else{ //DEFAULT Values
        if(animationID == ANIM_SWEEP_OUT) animationColor = TFT_BLACK;
        else animationColor = TFT_YELLOW;
    } 
}