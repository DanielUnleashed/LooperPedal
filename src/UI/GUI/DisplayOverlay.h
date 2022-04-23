#ifndef DISPLAY_OVERLAY_h
#define DISPLAY_OVERLAY_h

#include "UI/GUI/DisplayItem.h"

class DisplayOverlay : public DisplayItem{

    public:
    static const uint8_t NONE_ANIMATION = 0;
    static const uint8_t SWEEP_ANIMATION = 1;

    DisplayOverlay();
    void draw() override;

    uint8_t animationIndex = 0;
    void drawAnimation(uint8_t animationIndex);
};

#endif