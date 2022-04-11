#include "DebounceButton.h"

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
                buttonIsPressed = true;;
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