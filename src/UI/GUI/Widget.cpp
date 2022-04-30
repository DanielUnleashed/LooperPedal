#include "Widget.h"
#include "UI/MenuManager.h"
#include "UI/GUI/Taskbar.h"

std::vector<Widget*> Widget::displayedWidgets;
TaskHandle_t Widget::widgetEventHandle = NULL;
uint16_t Widget::tileSize, Widget::padX, Widget::padY;

uint8_t Widget::widgetEvent = NONE_EVENT;

int8_t Widget::holdingPosition = 0, Widget::previousHoldingPosition = 0, Widget::selectedWidget = 0, Widget::inWidgetSelection = 0;
bool Widget::isWidgetSelectionMode = false;

bool Widget::tileMapOverlap[TILES_X][TILES_Y];

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

    if(MenuManager::isLaunched){
        switchSelectionMode();
        holdingPosition = 0;
        selectedWidget = 0; //This should be changed!
    }
}

void Widget::startDraw(TFT_eSprite &canvas){
    //Maybe draw a background here?
    canvas.fillRect(0,0,width, height-TASKBAR_HEIGHT, TFT_BLACK);
}

void Widget::draw(){
    if(isWidgetSelectionMode){
        if(isSelected()) return;
        drawFilledRect(0,0,100,100,TFT_DARKGREY);
    }else{
        widgetDraw();
        if(isSelected()) drawRectangle(0,0,100,100, TFT_RED);
    }
}

void Widget::finalDraw(TFT_eSprite &canvas){
    if(isWidgetSelectionMode){
        Widget* w = displayedWidgets[selectedWidget];
        getTileOverlapMap();

        for(uint8_t x = w->tileX; x <w->tileX+w->sizeX; x++){
            for(uint8_t y = w->tileY; y <w->tileY+w->sizeY; y++){
                if(tileMapOverlap[x][y]) fillTile(canvas, x, y, TFT_RED);
                else fillTile(canvas, x, y, TFT_GREEN);
            }
        }
        displayedWidgets[selectedWidget]->drawRectangle(0,0,100,100,TFT_DARKGREEN);
    }
}

bool Widget::isSelected(){
    return displayedWidgets[selectedWidget]->widgetID == widgetID;
}

void Widget::switchSelectionMode(){
    widgetEvent = SWITCH_SELECTION_MODES;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(widgetEventHandle, &xHigherPriorityTaskWoken); 
}

void Widget::increaseCursor(){
    widgetEvent = INCREASE_CURSOR;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(widgetEventHandle, &xHigherPriorityTaskWoken); 
}

void Widget::decreaseCursor(){
    widgetEvent = DECREASE_CURSOR;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(widgetEventHandle, &xHigherPriorityTaskWoken); 
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

bool Widget::areTilesOverlapping(){
    bool tileMap[TILES_X][TILES_Y];
    memset(tileMap, 0, sizeof(tileMap));
    for(Widget* w : displayedWidgets){
        for(uint8_t i = 0; i < w->sizeX; i++){
            for(uint8_t j = 0; j < w->sizeY; j++){
                if(tileMap[w->tileX+i][w->tileY+j]) return true;
                else tileMap[w->tileX+i][w->tileY+j] = true;
            }
        }
    }
    return false;
}

bool Widget::getTileOverlapMap(){
    bool tileMap[TILES_X][TILES_Y];
    memset(tileMap, 0, sizeof(tileMap));
    memset(tileMapOverlap, 0, sizeof(tileMapOverlap));
    bool theresOverlap = false;
    for(Widget* w : displayedWidgets){
        for(uint8_t i = 0; i < w->sizeX; i++){
            for(uint8_t j = 0; j < w->sizeY; j++){
                if(tileMap[w->tileX+i][w->tileY+j]){
                    theresOverlap = true;
                    tileMapOverlap[w->tileX+i][w->tileY+j] = true;
                }else tileMap[w->tileX+i][w->tileY+j] = true;
            }
        }
    }
    return theresOverlap;
}

void Widget::redrawAll(){
    for(Widget* w : displayedWidgets) w->needsUpdate = true;
    displayedWidgets[0] -> redrawFromISR();
}

void Widget::startWidgets(){
    uint16_t distX = width/TILES_X;
    uint16_t distY = (height - TASKBAR_HEIGHT)/TILES_Y;
    tileSize = distX<distY ? distX : distY;

    Serial.printf("TileSize: %d\n", tileSize);

    padX = (width - tileSize*TILES_X)/2;
    padY = (height - TASKBAR_HEIGHT - tileSize*TILES_Y)/2;

    xTaskCreatePinnedToCore(widgetEventTask, "WidgetEvents", 10000, NULL, 5, &widgetEventHandle, 0);
}

void Widget::widgetEventTask(void* funcParams){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if(widgetEvent == NONE_EVENT) continue;
        else if(widgetEvent == INCREASE_CURSOR){
            if(isWidgetSelectionMode){
                holdingPosition++;
                if(holdingPosition >= (TILES_X-displayedWidgets[selectedWidget]->sizeX+1)*(TILES_Y-displayedWidgets[selectedWidget]->sizeY+1)) 
                    holdingPosition = 0;
                displayedWidgets[selectedWidget] -> recalculateRelativePoint();
                redrawAll();
            }else{
                displayedWidgets[selectedWidget] -> redrawFromISR();
                if(inWidgetSelection+1 < displayedWidgets[selectedWidget]->inWidgetSelectables){
                    inWidgetSelection++;
                }else{
                    if(selectedWidget+1 < displayedWidgets.size()) selectedWidget++;
                    else selectedWidget = 0;
                    inWidgetSelection = 0;
                }
                displayedWidgets[selectedWidget] -> redrawFromISR();
            }
        }else if(widgetEvent == DECREASE_CURSOR){
            if(isWidgetSelectionMode){
                holdingPosition--;
                if(holdingPosition < 0) 
                    holdingPosition = (TILES_X-displayedWidgets[selectedWidget]->sizeX+1)*(TILES_Y-displayedWidgets[selectedWidget]->sizeY+1) - 1;
                displayedWidgets[selectedWidget] -> recalculateRelativePoint();
                redrawAll();
            }else{
                displayedWidgets[selectedWidget] -> redrawFromISR();
                if(inWidgetSelection-1 >= 0){
                    inWidgetSelection--;
                }else{
                    if(selectedWidget-1 < 0) selectedWidget = displayedWidgets.size()-1;
                    else selectedWidget--;
                    inWidgetSelection = displayedWidgets[selectedWidget]->inWidgetSelectables-1;
                }
                displayedWidgets[selectedWidget] -> redrawFromISR();
            }
        }else if(widgetEvent == SWITCH_SELECTION_MODES){
            // If tiles are overlapping then don't change modes.
            if(isWidgetSelectionMode && areTilesOverlapping()) continue;

            // When entering selection mode, store the original position, so if 'Back' is pressed, it can return to the original position.
            if(!isWidgetSelectionMode)
                previousHoldingPosition = displayedWidgets[selectedWidget]->tileY*(TILES_X - displayedWidgets[selectedWidget]->sizeX + 1) + displayedWidgets[selectedWidget]->tileX;

            isWidgetSelectionMode = !isWidgetSelectionMode;
            int8_t taskbarIndex = MenuManager::getCurrentDisplay()->hasTaskbar();
            Taskbar* t;
            if(taskbarIndex >= 0) t = (Taskbar*)MenuManager::getCurrentDisplay()->getDisplayItem(taskbarIndex);

            if(isWidgetSelectionMode){
                Widget* sel = displayedWidgets[selectedWidget];
                holdingPosition = sel->tileY*(TILES_X - sel->sizeX + 1) + sel->tileX;

                if(taskbarIndex >= 0) { //If the taskbar exists then...
                    //Substitute all the current buttons and add the custom move functions.
                    t->saveAndRemoveButtons();
                    DebounceButton::saveAndRemoveButtons();
                    t->addButton("Dim.X", 0);
                    t->addButton("Dim.Y", 1);
                    t->addButton("Back", 3);

                    DebounceButton::addInterrupt(0, []{
                        displayedWidgets[selectedWidget] -> sizeX++;
                        if(displayedWidgets[selectedWidget]->sizeX > TILES_X) displayedWidgets[selectedWidget]->sizeX = 1;
                        displayedWidgets[selectedWidget] -> redrawFromISR();
                    });

                    DebounceButton::addInterrupt(1, []{
                        displayedWidgets[selectedWidget] -> sizeY++;
                        if(displayedWidgets[selectedWidget]->sizeY > TILES_Y) displayedWidgets[selectedWidget]->sizeY = 1;
                        displayedWidgets[selectedWidget] -> redrawFromISR();
                    });

                    DebounceButton::addInterrupt(3, []{
                        widgetEvent = UNDO_WIDGET_SELECTION;
                        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                        vTaskNotifyGiveFromISR(widgetEventHandle, &xHigherPriorityTaskWoken); 
                    });
                    t->forceRedraw();
                }
            }else{
                if(taskbarIndex >= 0) {
                    // Undo the change made above.
                    t->undoRemoveButtons();
                    DebounceButton::undoRemoveButtons();
                    t->forceRedraw();
                }
            }
            redrawAll();
        }else if(widgetEvent == UNDO_WIDGET_SELECTION){
            holdingPosition = previousHoldingPosition;
            displayedWidgets[selectedWidget] -> recalculateRelativePoint();
            switchSelectionMode();
        }
    }
}

Point Widget::transformRelativePoint(uint8_t pX, uint8_t pY){
    Point ret;
    if(pX < 100) ret.x = oX + sizeX*tileSize*pX/100;
    else ret.x = width-1;
    if(pY < 100) ret.y = oY + sizeY*tileSize*pY/100;
    else ret.y = height-1;
    return ret;
}

void Widget::recalculateRelativePoint(){
    tileY = holdingPosition/(TILES_X-sizeX+1);
    tileX = holdingPosition%(TILES_X-sizeX+1);
    oX = tileSize*tileX + padX;
    oY = tileSize*tileY + padY;
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

void Widget::fillTile(TFT_eSprite &canvas, uint8_t x, uint8_t y, uint16_t color){
    uint16_t oX = padX + x*tileSize;
    uint16_t oY = padY + y*tileSize;

    uint16_t mX = oX + tileSize-1;
    uint16_t mY = oY + tileSize-1;
    for(uint8_t i = 1; i < tileSize; i+=4){
        canvas.drawLine(oX + i, oY, oX, oY + i, color);
        canvas.drawLine(mX - i, mY, mX, mY - i, color);
    }
}