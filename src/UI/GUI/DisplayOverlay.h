#ifndef DISPLAY_OVERLAY_h
#define DISPLAY_OVERLAY_h

#include "UI/GUI/DisplayItem.h"

struct ANIMATION_DATA {
    uint8_t animationID;
    uint16_t animationColor;
    uint16_t posWaitTime;
    std::vector<void*> params;
};

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

    static const uint8_t ANIM_CIRCUMFERENCE = 5;
        static const uint8_t circumferenceRadius = 40;
        static const uint8_t pincelStroke = 4;
        static constexpr double circumferenceSpeed = 10;
        static constexpr double startAngle = HALF_PI; //Starts at the bottom

    static const uint8_t ANIM_TRIANGLE = 6;
        static const uint8_t rotAngle = 0;

    DisplayOverlay();
    void draw() override;
    void drawAnimation(uint8_t animationID);
    void drawMultipleAnimation(std::vector<uint8_t> animationQueue);
    void drawMultipleAnimation(std::vector<uint8_t> animationQueue, std::vector<uint16_t> animationQueuePalette);

    uint16_t diagonalRadius;

    private:
    uint8_t animationID = 0;
    uint16_t animationColor;
    void setPalette();

    bool hasMultipleAnimation = false;
    std::vector<uint8_t> animationQueue;
    std::vector<uint16_t> animationQueuePalette;
    uint8_t currentAnimationIndex = 0;

};

#endif