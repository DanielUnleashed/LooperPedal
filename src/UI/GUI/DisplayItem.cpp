#include "DisplayItem.h"

TFT_eSPI DisplayItem::tft = TFT_eSPI();
uint16_t DisplayItem::width, DisplayItem::height, DisplayItem::tileW, DisplayItem::tileH;

DisplayItem::DisplayItem(String name, uint8_t tx, uint8_t ty, uint8_t sx, uint8_t sy){
    itemName = name;
    tileX = tx;
    tileY = ty;
    sizeX = sx;
    sizeY = sy;
    oX = tileW*tileX;
    oY = tileH*tileY;
}

DisplayItem::~DisplayItem(){
    if(pressFunction) DebounceButton::removeInterrupt(inputPin);
}


// ***** DRAW FUNCTIONS *****
void DisplayItem::drawTiles(){
  // Draw tile squares.
  tft.drawRect(0,0,tft.width(),tft.height(), TFT_WHITE);

  //Line 0 has already been drawn.
  for(uint8_t i = 1; i < TILES_X; i++)
    tft.drawFastVLine(i*tileW, 0, height, TFT_WHITE);

  for(uint8_t i = 1; i < TILES_Y; i++)
    tft.drawFastHLine(0, i*tileH, width, TFT_WHITE);
}

void DisplayItem::drawRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileW*sX/100;
    uint16_t ly = sizeY*tileH*sY/100;

    tft.drawRect(transP.x, transP.y, lx, ly, color);
}

void DisplayItem::drawRoundRectangle(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileW*sX/100;
    uint16_t ly = sizeY*tileH*sY/100;

    tft.drawRoundRect(transP.x, transP.y, lx, ly, 4, color);
}

void DisplayItem::drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t fillColor){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileW*sX/100;
    uint16_t ly = sizeY*tileH*sY/100;

    tft.fillRect(transP.x, transP.y, lx, ly, fillColor);
}

void DisplayItem::drawFilledRect(uint8_t pX, uint8_t pY, uint8_t sX, uint8_t sY, uint16_t colorA, uint8_t colorB, bool drawHorizontal){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t lx = sizeX*tileW*sX/100;
    uint16_t ly = sizeY*tileH*sY/100;

    if(drawHorizontal) tft.fillRectVGradient(transP.x, transP.y, lx, ly, colorA, colorB);
    else tft.fillRectVGradient(transP.x, transP.y, lx, ly, colorA, colorB);
}

void DisplayItem::drawHLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t l = sizeX*tileW*length/100;
    tft.drawFastHLine(transP.x, transP.y, l, color);
}

void DisplayItem::drawVLine(uint8_t pX, uint8_t pY, uint8_t length, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    uint16_t l = sizeY*tileH*length/100;
    tft.drawFastVLine(transP.x, transP.y, l, color);
}

void DisplayItem::drawText(uint8_t pX, uint8_t pY, String text, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    tft.setTextColor(color);
    tft.setTextDatum(TL_DATUM); //Top left datum
    tft.drawString(text, transP.x, transP.y, 2);
}

void DisplayItem::drawCentreText(uint8_t pX, uint8_t pY, String text, uint16_t color){
    Point transP = transformRelativePoint(pX, pY);
    tft.setTextColor(color);
    tft.setTextDatum(MC_DATUM); //Medium center datum
    tft.drawString(text, transP.x, transP.y, 2);
}

void DisplayItem::addPressEvent(uint8_t pin, std::function<void(void)> func){
    pressFunction = func;
    inputPin = pin;
    if(!DebounceButton::addInterrupt(inputPin, pressFunction)) Utilities::debug("Failed to add ISR to %s\n", itemName.c_str());
}
void DisplayItem::addLEDTask(uint8_t LED_index, std::function<void(void)> func){
    LEDFunction = func;
}

Point DisplayItem::transformRelativePoint(uint8_t pX, uint8_t pY){
    Point ret;
    if(pX < 100) ret.x = oX + sizeX*tileW*pX/100;
    else ret.x = width-1;
    if(pY < 100) ret.y = oY + sizeY*tileH*pY/100;
    else ret.y = height-1;
    return ret;
}

void DisplayItem::startTFT(){
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    width = tft.width();
    height = tft.height();
    tileW = tft.width()/TILES_X;
    tileH = tft.height()/TILES_Y;

    Serial.printf("TFT(%dx%d tiles). w=%d  h=%d\n", TILES_X, TILES_Y, width, height);
}