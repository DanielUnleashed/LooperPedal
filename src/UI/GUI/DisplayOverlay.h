#ifndef DISPLAY_OVERLAY_h
#define DISPLAY_OVERLAY_h

#include "UI/GUI/DisplayItem.h"

class DisplayOverlay : public DisplayItem{

    public:
    static const uint8_t ANIM_NONE = 0;

    static const uint8_t ANIM_SWEEP_IN = 1;
    static const uint8_t ANIM_SWEEP_OUT = 2;
    static const uint8_t ANIM_SWEEP_IN_OUT = 0x80;
        static const uint8_t barWidth = 10;
        static constexpr double sweepSpeed = 2;
    
    static const uint8_t ANIM_CIRCLE = 4;
        static constexpr double circleSpeed = 2.5;

    // POLAR PLOTS CONSTANTS
    static const uint8_t outerPincelStroke = 5;
    static const uint8_t innerPlincelStroke = 3;
    static constexpr double plottingSpeed = 10;
    static const uint8_t plottingRadius = 40;

    static const uint8_t ANIM_CIRCUMFERENCE = 5;
    // ANIM_POLYGON | Nsides
    static const uint8_t ANIM_POLYGON = 0x40;

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

    bool drawNGon(uint8_t sides, double rotAngle, double startAngle);

};

#endif