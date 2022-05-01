#ifndef DEMO_WIDGET_h
#define DEMO_WIDGET_h

#include "Widget.h"

class DemoWidget : public Widget{

    public:
    DemoWidget(uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY);

    void widgetDraw() override;

};

#endif