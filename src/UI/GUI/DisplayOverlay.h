#ifndef DISPLAY_OVERLAY_h
#define DISPLAY_OVERLAY_h

#include "UI/GUI/DisplayItem.h"

class DisplayOverlay : public DisplayItem{

    public:
    static const uint8_t ANIM_NONE = 0;
    static const uint8_t ANIM_SWEEP = 1;
        static const uint8_t barWidth = 10;
        static const uint8_t sweepSpeed = 100;

    DisplayOverlay();
    void draw() override;

    uint8_t animationIndex = 0;
    void drawAnimation(uint8_t animationIndex);
};

#endif