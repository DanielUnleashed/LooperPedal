#include "DemoWidget.h"

#include "UI/MenuManager.h"

DemoWidget::DemoWidget(uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY) : Widget("DemoWidget", tileX, tileY, sizeX, sizeY, 1){
    addRotaryEvent(0, [this](bool in){
        if(in) increaseCursor();
        else decreaseCursor();
        redrawFromISR();
    });

    addRotaryButtonEvent(0, [this]{
        switchSelectionMode();
        redrawFromISR();
    });

    addButtonEvent(0, [this]{
        MenuManager::getCurrentDisplay() -> addItem(new DemoWidget(0,0,2,2));
        redrawFromISR();
    });
}

void DemoWidget::widgetDraw(){
    drawFilledRect(0,0,100,100,TFT_BLACK);
    drawFilledRect(0,0,100,100,TFT_DARKCYAN);
    drawCentreText(50,50,"Pollo",TFT_WHITE);
}