#include "DebounceButton.h"

DebounceButton::DebounceButton(uint8_t chipPin){
    pin = chipPin;
}

bool DebounceButton::updateState(){
    bool currentPinState = digitalRead(pin);
    uint32_t currentTime = millis();
    if(currentPinState != lastState){
        lastTimePressed = currentTime;
        lastState = currentPinState;
    }

    bool hasBeenUpdated = false;
    if((currentTime - lastTimePressed) > DEFAULT_DEBOUNCE_TIME){
        hasBeenUpdated = true;
        if(lastState == HIGH && currentPinState == LOW){
            buttonIsPressed = false;
        }
        if(lastState == LOW && currentPinState == HIGH){
            if((currentTime - doubleClickedTime) < DEFAULT_DOUBLE_CLICK_TIME){
                repeatedPressesCount++;
            }else{
                repeatedPressesCount = 1;
            }
            buttonIsPressed = true;;
            doubleClickedTime = currentTime;
        }
    }
    return hasBeenUpdated;
}

bool DebounceButton::buttonClicked(){
    // If the state hasn't been updated, then it returns false so that the code that comes
    // after this, doesn't execute.
    return updateState() && buttonIsPressed;
}

bool DebounceButton::buttonClicked(uint8_t timesPressed){
    return updateState() && (buttonIsPressed || (repeatedPressesCount >= timesPressed));
}

bool DebounceButton::buttonDoubleClicked(){
    return updateState() && (buttonIsPressed || (repeatedPressesCount > 1));
}