#include "DemoWidget.h"

DemoWidget::DemoWidget(uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY) : Widget("DemoWidget", tileX, tileY, sizeX, sizeY){
    addRotaryEvent(0, [this](bool in){
        if(!isSelectingTile) return;

        if(in) increaseCursor();
        else decreaseCursor();
        redraw();
    });

    addRotaryButtonEvent(0, [this]{
        Serial.println("hallo");
        isSelectingTile = !isSelectingTile;
        isWidgetSelected = !isWidgetSelected;
        redraw();
    });
}

void DemoWidget::widgetDraw(){
    drawFilledRect(0,0,100,100,TFT_BLACK);
    drawFilledRect(0,0,100,100,TFT_DARKCYAN);
    drawCentreText(50,50,"Pollo",TFT_WHITE);
}