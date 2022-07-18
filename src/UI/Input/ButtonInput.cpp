#include "ButtonInput.h"

ButtonInput::ButtonInput(){}

ButtonInput* ButtonInput::buttonLongPressWatch = NULL;

TaskHandle_t ButtonInput::buttonTaskHandle = NULL;

void ButtonInput::startButtons(){
    static bool started = false;
    if(started) return;
    xTaskCreatePinnedToCore(longPressTimeTask, "LongPressButton Task", 10000, NULL, 5, &buttonTaskHandle, 0);
    started = true;
}

bool ButtonInput::updateState(){
    bool currentPinState = fetchInput();
    uint32_t currentTime = millis();
    //Serial.printf("Curr:%d  Last:%d  Curr:%d Last:%d -> ", currentPinState, lastState, currentTime, lastTimePressed);
    if(currentPinState != lastState){
        bool ret = false;
        if((currentTime - lastTimePressed) > DEFAULT_DEBOUNCE_TIME){
            lastTimePressed = currentTime;
            if(lastState == HIGH && currentPinState == LOW){
                buttonIsPressed = false;
                //Serial.println("RELEASED");
                ret = true;
            }
            if(lastState == LOW && currentPinState == HIGH){
                if((currentTime - doubleClickedTime) < DEFAULT_DOUBLE_CLICK_TIME){
                    repeatedPressesCount++;
                }else{
                    repeatedPressesCount = 1;
                }
                buttonIsPressed = true;
                doubleClickedTime = currentTime;
                //Serial.println("CLICK");
                ret = true;
            }
        }
        lastState = currentPinState;
        return ret;
    }
    return false;
}

void ButtonInput::longPressTimeTask(void* funcParams){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        uint8_t i;
        for(i = 0; i < 5; i++){
            delay(200);
            if(buttonLongPressWatch==NULL || !buttonLongPressWatch->isPressed()){
                break;
            }
        }

        if(i == 5 && buttonLongPressWatch!=NULL){
            // This function could be null if another program had taken the control of the input. Then the 
            // function would be deleted.
            for(uint8_t i = 0; i < buttonLongPressWatch->eventFunction.size(); i++){
                ButtonFunction ev = buttonLongPressWatch->eventFunction[i];
                if(ev.func) ev.func();
            }
            
        }
        buttonLongPressWatch = NULL;
    }
}

void ButtonInput::eventEvaluation(){
    bool input = clicked();
    for(uint8_t i = 0; i < eventFunction.size(); i++){
        ButtonFunction ev = eventFunction[i];
        //Continue if there is no more functions.
        if(!ev.func) continue; 

        //Switch depending on the type of input assigned to the function.
        if(ev.functionalEvent == CLICK){
            if(input) ev.func();
        }else if(ev.functionalEvent == LONG_PRESS) {
            //Continue if a long press is being processed or if the button is released.
            if((buttonLongPressWatch!=NULL && pin == buttonLongPressWatch->pin) || !input) continue;

            buttonLongPressWatch = this;

            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(buttonTaskHandle, &xHigherPriorityTaskWoken); 
        }
    }
}

bool ButtonInput::clicked(){
    // If the state hasn't been updated, then it returns false so that the code that comes
    // after this, doesn't execute.
    return updateState() && buttonIsPressed;
}

bool ButtonInput::isPressed(){
    updateState();
    return buttonIsPressed;
}

bool ButtonInput::released(){
    return updateState() && !buttonIsPressed;
}

bool ButtonInput::clicked(uint8_t timesPressed){
    return updateState() && (buttonIsPressed || (repeatedPressesCount >= timesPressed));
}

bool ButtonInput::doubleClicked(){
    return updateState() && (buttonIsPressed || (repeatedPressesCount > 1));
}

bool ButtonInput::twoButtonsClicked(ButtonInput* otherButton){
    return (millis()-otherButton->lastTimePressed) < DEFAULT_DEBOUNCE_TIME;
}

void ButtonInput::clearLongPressButton(){
    buttonLongPressWatch = NULL;
}

bool ButtonInput::addButtonFunction(std::function<void(void)> func, uint8_t mode){
    for(uint8_t i = 0; i < eventFunction.size(); i++){
        if(eventFunction[i].functionalEvent == mode){
            Utilities::debug("Button is already in use with mode %d", mode);
            return false;
        }
    }
    eventFunction.push_back({func, mode});
    return true;
}

void ButtonInput::saveAndRemoveButton(){
    previousEventFunction = eventFunction;
    eventFunction.clear();
}

void ButtonInput::undoRemoveButton(){
    eventFunction.clear();
    eventFunction = previousEventFunction;
}

void ButtonInput::clearEvents(){
    eventFunction.clear();
}