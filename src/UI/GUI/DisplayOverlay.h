#ifndef DISPLAY_OVERLAY_h
#define DISPLAY_OVERLAY_h

#include "UI/GUI/DisplayItem.h"

class DisplayOverlay : public DisplayItem{

    public:
    // Multiple animations must start with a 1 in binary.
    static const uint8_t ANIM_NONE = 0;

    static const uint8_t ANIM_SWEEP_IN = 1;
    static const uint8_t ANIM_SWEEP_OUT = 2;
    static const uint8_t ANIM_SWEEP_IN_OUT = 0x81;
        static const uint8_t barWidth = 10;
        static constexpr double sweepSpeed = 2;
    
    static const uint8_t ANIM_CIRCLE = 4;
        static constexpr double circleSpeed = 2.5;

    DisplayOverlay();
    void draw() override;
    void drawAnimation(uint8_t animationID);
    void drawMultipleAnimation(std::vector<uint8_t> animationQueue);

    uint16_t diagonalRadius;

    private:
    uint8_t animationID = 0;
    uint16_t animationColor;
    void setPalette();

    bool hasMultipleAnimation = false;
    std::vector<uint8_t> animationQueue;
    uint8_t currentAnimationIndex = 0;

};

#endif