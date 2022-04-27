#include "DisplayItem.h"

uint16_t DisplayItem::width, DisplayItem::height;
uint16_t DisplayItem::tileSize, DisplayItem::padX, DisplayItem::padY;

DisplayItem::DisplayItem(String name, uint8_t tx, uint8_t ty, uint8_t sx, uint8_t sy){
    itemName = name;
    tileX = tx;
    tileY = ty;
    sizeX = sx;
    sizeY = sy;

    oX = tileSize*tileX + padX;
    oY = tileSize*tileY + padY;
}

DisplayItem::DisplayItem(String n){
    itemName = n;
}

DisplayItem::~DisplayItem(){
    if(pressFunction) DebounceButton::removeInterrupt(inputPin);
}

// ***** DRAW FUNCTIONS *****
void DisplayItem::drawGrid(){
    // Draw tile squares.
    canvas->drawRect(padX,padY, tileSize*TILES_X, tileSize*TILES_Y, TFT_WHITE);
    //Line 0 has already been drawn.
    for(uint8_t i = 1; i < TILES_Y; i++)
        canvas->drawFastHLine(padX, padY+i*tileSize, tileSize*TILES_X, TFT_WHITE);
    for(uint8_t i = 1; i < TILES_X; i++)
        canvas->drawFastVLine(padX+i*tileSize, padY, tileSize*TILES_Y, TFT_WHITE);
}

void DisplayItem::drawRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileSize*sX/100;
    uint16_t ly = sizeY*tileSize*sY/100;

    canvas->drawRect(transP.x, transP.y, lx, ly, color);
}

void DisplayItem::drawRoundRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileSize*sX/100;
    uint16_t ly = sizeY*tileSize*sY/100;

    canvas->drawRoundRect(transP.x, transP.y, lx, ly, 4, color);
}

void DisplayItem::drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t fillColor){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileSize*sX/100;
    uint16_t ly = sizeY*tileSize*sY/100;

    canvas->fillRect(transP.x, transP.y, lx, ly, fillColor);
}

void DisplayItem::drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t colorA, uint16_t colorB, bool drawHorizontal){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileSize*sX/100;
    uint16_t ly = sizeY*tileSize*sY/100;

    if(drawHorizontal) canvas->fillRectHGradient(transP.x, transP.y, lx, ly, colorA, colorB);
    else canvas->fillRectVGradient(transP.x, transP.y, lx, ly, colorA, colorB);
}

void DisplayItem::drawHLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t l = sizeX*tileSize*length/100;
    canvas->drawFastHLine(transP.x, transP.y, l, color);
}

void DisplayItem::drawVLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t l = sizeY*tileSize*length/100;
    canvas->drawFastVLine(transP.x, transP.y, l, color);
}

void DisplayItem::drawText(uint8_t pX, uint8_t pY, String text, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    canvas->setTextColor(color);
    canvas->setTextDatum(TL_DATUM); //Top left datum
    canvas->drawString(text, transP.x, transP.y, 2);
}

void DisplayItem::drawCentreText(uint8_t pX, uint8_t pY, String text, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    canvas->setTextColor(color);
    canvas->setTextDatum(MC_DATUM); //Medium center datum
    canvas->drawString(text, transP.x, transP.y, 2);
}

void DisplayItem::pushSprite(TFT_eSprite &sp, uint16_t x, uint16_t y){
    sp.pushToSprite(canvas, x, y);
}

void DisplayItem::addPressEvent(uint8_t pin, std::function<void(void)> func){
    pressFunction = func;
    inputPin = pin;
    if(!DebounceButton::addInterrupt(inputPin, pressFunction)) Utilities::debug("Failed to add ISR to %s\n", itemName.c_str());
}

Point DisplayItem::transformRelativePoint(uint8_t pX, uint8_t pY){
    Point ret;
    if(pX < 100) ret.x = oX + sizeX*tileSize*pX/100;
    else ret.x = width-1;
    if(pY < 100) ret.y = oY + sizeY*tileSize*pY/100;
    else ret.y = height-1;
    return ret;
}

void DisplayItem::startDisplayItems(uint16_t w, uint16_t h){
    width = w;
    height = h;

    uint16_t distX = width/TILES_X;
    uint16_t distY = (height - TASKBAR_HEIGHT)/TILES_Y;
    tileSize = distX<distY ? distX : distY;

    padX = (width - tileSize*TILES_X)/2;
    padY = (height - TASKBAR_HEIGHT - tileSize*TILES_Y)/2;
}

void DisplayItem::attachRedrawHandler(TaskHandle_t h){
    redrawHandle = h;
}

void DisplayItem::startAnimation(){
    startAnimationTime = micros();
    redraw();
}

void DisplayItem::render(TFT_eSprite &c){
    canvas = &c;
    draw();
}

void DisplayItem::redraw(){
    needsUpdate = true;
    if(redrawHandle != NULL) xTaskNotifyGive(redrawHandle);
    else Utilities::error("RedrawHandle is null in %s!\n", itemName.c_str());
}

void DisplayItem::endAnimation(){
    needsUpdate = false;
}

bool DisplayItem::needsToRedraw(){
    return needsUpdate;
}

void DisplayItem::forceRedraw(){
    needsUpdate = true;
}

uint32_t DisplayItem::getTickTime(){
    return (micros() - startAnimationTime)*FPS_DESIGN/SCREEN_FPS/10000.0;
}