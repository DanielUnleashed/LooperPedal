#include "DemoWidget.h"

#include "UI/MenuManager.h"

DemoWidget::DemoWidget(uint8_t tileX, uint8_t tileY, uint8_t sizeX, uint8_t sizeY) : Widget("Demo", tileX, tileY, sizeX, sizeY, 1){
}

void DemoWidget::widgetDraw(){
    drawFilledRect(0,0,100,100,TFT_DARKCYAN);
    drawCentreText(50,50,"Demo",TFT_WHITE);
}

void DemoWidget::selectionFunctions(uint8_t selection){

}