#include "DebounceButton.h"
#include "UI/MenuManager.h"

DebounceButton* DebounceButton::systemButtons[TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS];

// Maybe implement with a loop with https://stackoverflow.com/questions/11081573/passing-a-variable-as-a-template-argument
void DebounceButton::init(){
    startButtons();

    for(uint8_t i = 0; i < TOTAL_BUTTONS; i++){
        systemButtons[i] = new DebounceButton(PUSH_BUTTON[i]);
    }
    for(uint8_t i = 0; i < TOTAL_ROTARY_BUTTONS; i++){
        systemButtons[TOTAL_BUTTONS+i] = new DebounceButton(ROTARY_BUTTONS[i]);
    }

    attachInterrupt(systemButtons[0]->pin, &ISR_BUTTON<0>, CHANGE);
    attachInterrupt(systemButtons[1]->pin, &ISR_BUTTON<1>, CHANGE);
    attachInterrupt(systemButtons[2]->pin, &ISR_BUTTON<2>, CHANGE);
    attachInterrupt(systemButtons[3]->pin, &ISR_BUTTON<3>, CHANGE);
    attachInterrupt(systemButtons[4]->pin, &ISR_BUTTON<4>, CHANGE);
    attachInterrupt(systemButtons[5]->pin, &ISR_BUTTON<5>, CHANGE);

    // Button from the rotary encoder.
    attachInterrupt(systemButtons[6]->pin, &ISR_BUTTON<6>, CHANGE);
}

template <int interrupt>
void IRAM_ATTR DebounceButton::ISR_BUTTON(){
    systemButtons[interrupt]->eventEvaluation();
}

void DebounceButton::saveAndRemoveButtons(){
    ButtonInput::clearLongPressButton();
    for(uint8_t i = 0; i < TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS; i++){
        systemButtons[i]->saveAndRemoveButton();
    }
}

void DebounceButton::undoRemoveButtons(){
    for(uint8_t i = 0; i < TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS; i++){
        systemButtons[i]->undoRemoveButton();
    }
}

bool DebounceButton::addInterrupt(uint8_t buttonIndex, std::function<void(void)> func){
    return addInterrupt(buttonIndex, func, CLICK);
}

bool DebounceButton::addInterrupt(uint8_t buttonIndex, String tagName, std::function<void(void)> func){
    Display* d = MenuManager::getCurrentDisplay();
    if(d!=NULL){
        Taskbar* t = d->getTaskbar();
        if(t!=NULL) t->addButton(buttonIndex, tagName);
    }
    return addInterrupt(buttonIndex, func, CLICK);
}

bool DebounceButton::addInterrupt(uint8_t buttonIndex, String tagName, std::function<void(void)> func, uint8_t mode){
    Display* d = MenuManager::getCurrentDisplay();
    if(d!=NULL){
        Taskbar* t = d->getTaskbar();
        if(t!=NULL) t->addButton(buttonIndex, tagName);
    }
    return addInterrupt(buttonIndex, func, mode);
}

bool DebounceButton::addRotaryInterrupt(uint8_t buttonIndex, std::function<void(void)> func){
    return addInterrupt(TOTAL_BUTTONS+buttonIndex, func, CLICK);
}

bool DebounceButton::addRotaryInterrupt(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode){
    return addInterrupt(TOTAL_BUTTONS+buttonIndex, func, mode);
}

bool DebounceButton::addInterrupt(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode){
    if(buttonIndex > TOTAL_BUTTONS) Utilities::debug("%sButton %d is over the number of buttons!\n", 
        buttonIndex>TOTAL_BUTTONS?"Rotary":"",  
        buttonIndex>TOTAL_BUTTONS?buttonIndex-TOTAL_BUTTONS:buttonIndex);

    bool ok = systemButtons[buttonIndex]->addButtonFunction(func, mode);
    if(!ok) Serial.printf(" (Button %d)", buttonIndex);
    return ok;
}

bool DebounceButton::addMultipleInterrupt(uint8_t* buttonIndexes, std::function<void(void)> func){
    if(sizeof(buttonIndexes) == 0) Utilities::debug("There are no buttons to add\n");
    bool allSet = true;
    for(uint8_t i = 0; i < sizeof(buttonIndexes); i++){
        if(!addInterrupt(buttonIndexes[i], func)){
            allSet = false;
            Utilities::debug("Button %d couldn't add its interrupt!\n", i);
        } 
    }
    return allSet;
}

bool DebounceButton::clearAll(){
    /*uint8_t clearArray[4];
    for(uint8_t i = 0; i < 4; i++) clearArray[i] = i;
    clearMultipleInterrupt(clearArray);*/
    for(uint8_t i = 0; i < TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS; i++){
        systemButtons[i]->clearEvents();
    }
    return true;
}

bool DebounceButton::clearMultipleInterrupt(uint8_t* buttonIndexes){
    if(sizeof(buttonIndexes) == 0) Utilities::debug("There are no button to clear\n");
    bool allSet = true;
    for(uint8_t i = 0; i < sizeof(buttonIndexes); i++){
        if(!removeInterrupt(buttonIndexes[i])){
            allSet = false;
            Utilities::debug("Button %d couldn't remove its interrupt!\n", i);
        } 
    }
    return allSet;
}

DebounceButton::DebounceButton(uint8_t chipPin):ButtonInput(){
    pin = chipPin;
    pinMode(chipPin, INPUT);
}

bool DebounceButton::fetchInput(){
    return digitalRead(pin);
}