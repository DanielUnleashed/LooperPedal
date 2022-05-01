#include "DemoWidget.h"

#include "UI/MenuManager.h"

DemoWidget::DemoWidget(uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY) : Widget("DemoWidget", tileX, tileY, sizeX, sizeY, 1){
    addRotaryEvent(0, [this](bool in){
        if(in) increaseCursor();
        else decreaseCursor();
    });

    addRotaryButtonEvent(0, [this]{
        switchSelectionMode();
    });

    addButtonEvent(0, [this]{
        if(!Widget::isWidgetSelectionMode) MenuManager::getCurrentDisplay() -> addItem(new DemoWidget(0,0,2,2));
        else Utilities::debug("First drop the item!\n");
    }, "New");
}

void DemoWidget::widgetDraw(){
    drawFilledRect(0,0,100,100,TFT_DARKCYAN);
    drawCentreText(50,50,"Pollo",TFT_WHITE);
}