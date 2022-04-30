#include "DisplayItem.h"
#include "UI/MenuManager.h"

uint16_t DisplayItem::width, DisplayItem::height;

DisplayItem::DisplayItem(String n){
    itemName = n;
}

void DisplayItem::attachEvents(){
    for(ButtonEvent ev : buttonEvents) DebounceButton::addInterrupt(ev.pin, ev.func);
    for(RotaryEncoderEvent ev : rotaryEvents) RotaryEncoder::addInterrupt(ev.pin, ev.func);
    for(ButtonEvent ev : rotaryButtonEvents) RotaryEncoder::addButtonInterrupt(ev.pin, ev.func);
}

void DisplayItem::addButtonEvent(uint8_t buttonIndex, std::function<void()> func){
    ButtonEvent ev = {buttonIndex, func};
    buttonEvents.push_back(ev);
}

void DisplayItem::addRotaryEvent(uint8_t rotatoryIndex, std::function<void(bool incr)> func){
    RotaryEncoderEvent ev = {rotatoryIndex, func};
    rotaryEvents.push_back(ev);
}

void DisplayItem::addRotaryButtonEvent(uint8_t buttonIndex, std::function<void(void)> func){
    ButtonEvent ev = {buttonIndex, func};
    rotaryButtonEvents.push_back(ev);
}

void DisplayItem::startDisplayItems(uint16_t w, uint16_t h){
    width = w;
    height = h;
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
    MenuManager::wakeUpDrawTask();
}

void DisplayItem::redrawFromISR(){
    needsUpdate = true;
    MenuManager::wakeUpDrawTaskFromISR();
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