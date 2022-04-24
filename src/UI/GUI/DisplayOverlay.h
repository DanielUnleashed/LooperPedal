#ifndef DISPLAY_OVERLAY_h
#define DISPLAY_OVERLAY_h

#include "UI/GUI/DisplayItem.h"

class DisplayOverlay : public DisplayItem{

    public:
    static const uint8_t ANIM_NONE = 0;
    static const uint8_t ANIM_SWEEP = 1;
        static const uint8_t barWidth = 10;
        static constexpr double sweepSpeed = 2;
    static const uint8_t ANIM_CIRCLE = 2;
        static constexpr double circleSpeed = 2.5;

    DisplayOverlay();
    void draw() override;

    uint8_t animationIndex = 0;
    void drawAnimation(uint8_t animationIndex);

    uint16_t diagonalRadius;
};

#endif