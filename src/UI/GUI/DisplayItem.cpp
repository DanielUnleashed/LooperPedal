#include "DisplayItem.h"

TFT_eSPI DisplayItem::tft = TFT_eSPI();
uint16_t DisplayItem::width, DisplayItem::height, DisplayItem::tileW, DisplayItem::tileH;

DisplayItem::DisplayItem(String name, uint8_t tx, uint8_t ty, uint8_t sx, uint8_t sy){
    itemName = name;
    tileX = tx;
    tileY = ty;
    sizeX = sx;
    sizeY = sy;
}

DisplayItem::~DisplayItem(){
    DebounceButton::removeInterrupt(inputPin);
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


void DisplayItem::addPressEvent(uint8_t pin, std::function<void(void)> func){
    pressFunction = func;
    inputPin = pin;
    if(!DebounceButton::addInterrupt(inputPin, pressFunction)) Utilities::debug("Failed to add ISR to %s\n", itemName.c_str());
}
void DisplayItem::addLEDTask(uint8_t LED_index, std::function<void(void)> func){
    LEDFunction = func;
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