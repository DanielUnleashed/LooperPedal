#include "DebounceButton.h"

DebounceButton* DebounceButton::systemButtons[TOTAL_BUTTONS];
std::function<void(void)> IRAM_ATTR DebounceButton::ISREvents[TOTAL_BUTTONS];

// Maybe implement with a loop with https://stackoverflow.com/questions/11081573/passing-a-variable-as-a-template-argument
void DebounceButton::init(){
    for(uint8_t i = 0; i < TOTAL_BUTTONS; i++){
        systemButtons[i] = new DebounceButton(PUSH_BUTTON[i]); 
        ISREvents[i] = {};
    }

    attachInterrupt(systemButtons[0]->pin, &ISR_BUTTON<0>, CHANGE);
    attachInterrupt(systemButtons[1]->pin, &ISR_BUTTON<1>, CHANGE);
    attachInterrupt(systemButtons[2]->pin, &ISR_BUTTON<2>, CHANGE);
    attachInterrupt(systemButtons[3]->pin, &ISR_BUTTON<3>, CHANGE);
    attachInterrupt(systemButtons[4]->pin, &ISR_BUTTON<4>, CHANGE);
    attachInterrupt(systemButtons[5]->pin, &ISR_BUTTON<5>, CHANGE);
}

template <int interrupt>
void IRAM_ATTR DebounceButton::ISR_BUTTON(){
    // Will check if the ISR has been added and if the button was clicked
    if(ISREvents[interrupt] && systemButtons[interrupt] -> clicked()) ISREvents[interrupt](); 
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

bool DebounceButton::addInterrupt(uint8_t buttonIndex, std::function<void(void)> func){
    if(buttonIndex > TOTAL_BUTTONS) Utilities::debug("Button %d is out of the array!\n", buttonIndex);
    if(ISREvents[buttonIndex]){
        Utilities::debug("Button %d is already in use\n", buttonIndex);
        return false;
    }else{
        ISREvents[buttonIndex] = func;
        return true;
    }
}

bool DebounceButton::removeInterrupt(uint8_t buttonIndex){
    ISREvents[buttonIndex] = {};
    return !ISREvents[buttonIndex];
}

DebounceButton::DebounceButton(uint8_t chipPin){
    pin = chipPin;
    pinMode(chipPin, INPUT);
}

bool DebounceButton::updateState(){
    bool currentPinState = digitalRead(pin);
    uint32_t currentTime = millis();
    //Serial.printf("Curr:%d  Last:%d  Curr:%d Last:%d\n", currentPinState, lastState, currentTime, lastTimePressed);
    if(currentPinState != lastState){
        if((currentTime - lastTimePressed) > DEFAULT_DEBOUNCE_TIME){
            lastTimePressed = currentTime;
            if(lastState == HIGH && currentPinState == LOW){
                buttonIsPressed = false;
                return true;
            }
            if(lastState == LOW && currentPinState == HIGH){
                if((currentTime - doubleClickedTime) < DEFAULT_DOUBLE_CLICK_TIME){
                    repeatedPressesCount++;
                }else{
                    repeatedPressesCount = 1;
                }
                buttonIsPressed = true;
                doubleClickedTime = currentTime;
                return true;
            }
        }
        lastState = currentPinState;
    }
    return false;
}

bool DebounceButton::clicked(){
    // If the state hasn't been updated, then it returns false so that the code that comes
    // after this, doesn't execute.
    return updateState() && buttonIsPressed;
}

bool DebounceButton::clicked(uint8_t timesPressed){
    return updateState() && (buttonIsPressed || (repeatedPressesCount >= timesPressed));
}

bool DebounceButton::doubleClicked(){
    return updateState() && (buttonIsPressed || (repeatedPressesCount > 1));
}

bool DebounceButton::twoButtonsClicked(uint8_t otherButton){
    return (millis()-systemButtons[otherButton]->lastTimePressed) < DEFAULT_DEBOUNCE_TIME;
}
