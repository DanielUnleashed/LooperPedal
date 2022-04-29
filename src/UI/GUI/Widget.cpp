#include "Widget.h"

std::vector<Widget*> Widget::displayedWidgets;
uint16_t Widget::tileSize, Widget::padX, Widget::padY;

int8_t Widget::holdingPosition = 0, Widget::selectedWidget = 0, Widget::inWidgetSelection = 0;
bool Widget::isWidgetSelectionMode = false;

Widget::Widget(String name, uint8_t tx, uint8_t ty, uint8_t sx, uint8_t sy, uint8_t iW) : DisplayItem("Widget"){
    static uint16_t widgetCounter = 0;
    widgetID = widgetCounter++;

    widgetName = name;
    tileX = tx;
    tileY = ty;
    sizeX = sx;
    sizeY = sy;

    inWidgetSelectables = iW;

    oX = tileSize*tileX + padX;
    oY = tileSize*tileY + padY;
}

void Widget::draw(){
    if(isWidgetSelectionMode && isSelected()){
        if(holdingPosition >= (TILES_X-sizeX+1)*(TILES_Y-sizeY+1)) holdingPosition = 0;
        if(holdingPosition < 0) holdingPosition = (TILES_X-sizeX+1)*(TILES_Y-sizeY+1) - 1;

        drawFilledRect(0,0,100,100,TFT_BLACK);
        tileY = holdingPosition/(TILES_X-sizeX+1);
        tileX = holdingPosition%(TILES_X-sizeX+1);
        oX = tileSize*tileX + padX;
        oY = tileSize*tileY + padY;
        drawRectangle(0,0,100,100,TFT_GREEN);
    }else{
        widgetDraw();
        if(isSelected()) drawRectangle(0,0,100,100, TFT_RED);
    }
}

bool Widget::isSelected(){
    return displayedWidgets[selectedWidget]->widgetID == widgetID;
}

void Widget::switchSelectionMode(){
    isWidgetSelectionMode = !isWidgetSelectionMode;
    if(isWidgetSelectionMode){
        Widget* sel = displayedWidgets[selectedWidget];
        holdingPosition = sel->tileY*TILES_X + sel->tileX;
    }
}

void Widget::increaseCursor(){
    if(isWidgetSelectionMode) holdingPosition++;
    else{
        if(inWidgetSelection+1 < displayedWidgets[selectedWidget]->inWidgetSelectables){
            inWidgetSelection++;
        }else{
            if(selectedWidget+1 < displayedWidgets.size()) selectedWidget++;
            else selectedWidget = 0;
            inWidgetSelection = 0;
        }
    }
}

void Widget::decreaseCursor(){
    if(isWidgetSelectionMode) holdingPosition--;
    else{
        if(inWidgetSelection-1 >= 0){
            inWidgetSelection--;
        }else{
            if(selectedWidget-1 < 0) selectedWidget = displayedWidgets.size()-1;
            else selectedWidget--;
            inWidgetSelection = displayedWidgets[selectedWidget]->inWidgetSelectables-1;
        }
    }
}

void Widget::addWidget(Widget* w){
    displayedWidgets.push_back(w);
}

void Widget::removeWidget(Widget* in){
    for(auto it = begin(displayedWidgets); it != end(displayedWidgets); it++){
        if((*it)->widgetID == in->widgetID)
            displayedWidgets.erase(it);
    }
}

void Widget::clearWidgets(){
    displayedWidgets.clear();
}

void Widget::sortDisplayedWidgetsList(){
    std::sort(displayedWidgets.begin(), displayedWidgets.end(), [](Widget* w1, Widget* w2)->bool{
        uint16_t length1 = w1->tileY * TILES_X + w1->tileX;
        uint16_t length2 = w2->tileY * TILES_X + w2->tileX;
        return length1 < length2;
    });
}

void Widget::startWidgets(){
    uint16_t distX = width/TILES_X;
    uint16_t distY = (height - TASKBAR_HEIGHT)/TILES_Y;
    tileSize = distX<distY ? distX : distY;

    padX = (width - tileSize*TILES_X)/2;
    padY = (height - TASKBAR_HEIGHT - tileSize*TILES_Y)/2;
}

Point Widget::transformRelativePoint(uint8_t pX, uint8_t pY){
    Point ret;
    if(pX < 100) ret.x = oX + sizeX*tileSize*pX/100;
    else ret.x = width-1;
    if(pY < 100) ret.y = oY + sizeY*tileSize*pY/100;
    else ret.y = height-1;
    return ret;
}

// ***** DRAW FUNCTIONS *****
void Widget::drawGrid(){
    // Draw tile squares.
    canvas->drawRect(padX,padY, tileSize*TILES_X, tileSize*TILES_Y, TFT_WHITE);
    //Line 0 has already been drawn.
    for(uint8_t i = 1; i < TILES_Y; i++)
        canvas->drawFastHLine(padX, padY+i*tileSize, tileSize*TILES_X, TFT_WHITE);
    for(uint8_t i = 1; i < TILES_X; i++)
        canvas->drawFastVLine(padX+i*tileSize, padY, tileSize*TILES_Y, TFT_WHITE);
}

void Widget::drawRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileSize*sX/100;
    uint16_t ly = sizeY*tileSize*sY/100;

    canvas->drawRect(transP.x, transP.y, lx, ly, color);
}

void Widget::drawRoundRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileSize*sX/100;
    uint16_t ly = sizeY*tileSize*sY/100;

    canvas->drawRoundRect(transP.x, transP.y, lx, ly, 4, color);
}

void Widget::drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t fillColor){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileSize*sX/100;
    uint16_t ly = sizeY*tileSize*sY/100;

    canvas->fillRect(transP.x, transP.y, lx, ly, fillColor);
}

void Widget::drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t colorA, uint16_t colorB, bool drawHorizontal){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileSize*sX/100;
    uint16_t ly = sizeY*tileSize*sY/100;

    if(drawHorizontal) canvas->fillRectHGradient(transP.x, transP.y, lx, ly, colorA, colorB);
    else canvas->fillRectVGradient(transP.x, transP.y, lx, ly, colorA, colorB);
}

void Widget::drawHLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t l = sizeX*tileSize*length/100;
    canvas->drawFastHLine(transP.x, transP.y, l, color);
}

void Widget::drawVLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t l = sizeY*tileSize*length/100;
    canvas->drawFastVLine(transP.x, transP.y, l, color);
}

void Widget::drawText(uint8_t pX, uint8_t pY, String text, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    canvas->setTextColor(color);
    canvas->setTextDatum(TL_DATUM); //Top left datum
    canvas->drawString(text, transP.x, transP.y, 2);
}

void Widget::drawCentreText(uint8_t pX, uint8_t pY, String text, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    canvas->setTextColor(color);
    canvas->setTextDatum(MC_DATUM); //Medium center datum
    canvas->drawString(text, transP.x, transP.y, 2);
}

void Widget::pushSprite(TFT_eSprite &sp, uint16_t x, uint16_t y){
    sp.pushToSprite(canvas, x, y);
}