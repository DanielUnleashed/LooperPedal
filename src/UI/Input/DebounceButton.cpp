#include "DebounceButton.h"

DebounceButton* DebounceButton::systemButtons[TOTAL_BUTTONS];
IRAM_ATTR std::vector<ButtonFunction> DebounceButton::ISREvents[TOTAL_BUTTONS];
IRAM_ATTR std::vector<ButtonFunction> DebounceButton::previousISREvents[TOTAL_BUTTONS];

ButtonEvent DebounceButton::buttonLongPressWatch = {0xFF, {}};

TaskHandle_t DebounceButton::buttonTaskHandle = NULL;

// Maybe implement with a loop with https://stackoverflow.com/questions/11081573/passing-a-variable-as-a-template-argument
void DebounceButton::init(){
    for(uint8_t i = 0; i < TOTAL_BUTTONS; i++){
        systemButtons[i] = new DebounceButton(PUSH_BUTTON[i]); 
    }

    attachInterrupt(systemButtons[0]->pin, &ISR_BUTTON<0>, CHANGE);
    attachInterrupt(systemButtons[1]->pin, &ISR_BUTTON<1>, CHANGE);
    attachInterrupt(systemButtons[2]->pin, &ISR_BUTTON<2>, CHANGE);
    attachInterrupt(systemButtons[3]->pin, &ISR_BUTTON<3>, CHANGE);
    attachInterrupt(systemButtons[4]->pin, &ISR_BUTTON<4>, CHANGE);
    attachInterrupt(systemButtons[5]->pin, &ISR_BUTTON<5>, CHANGE);

    xTaskCreatePinnedToCore(longPressTimeTask, "WidgetEvents", 10000, NULL, 5, &buttonTaskHandle, 0);
}

template <int interrupt>
void IRAM_ATTR DebounceButton::ISR_BUTTON(){
    std::vector<ButtonFunction> eventList = ISREvents[interrupt];
    for(uint8_t i = 0; i < eventList.size(); i++){
        ButtonFunction ev = eventList[i];
        //Continue if there is no function.
        if(!ev.func) continue; 

        //Switch depending on the type of input assigned to the function.
        switch(ev.functionalEvent){
            case CLICK: if(systemButtons[interrupt]->clicked()) ev.func(); break;
            case LONG_PRESS:{
                //Continue if a long press is being processed.
                if(interrupt == buttonLongPressWatch.pin) continue;

                buttonLongPressWatch = {interrupt, ev.func};
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                vTaskNotifyGiveFromISR(buttonTaskHandle, &xHigherPriorityTaskWoken); 
                break;
            }
        }
    }
}

void DebounceButton::longPressTimeTask(void* funcParams){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(!systemButtons[buttonLongPressWatch.pin]->clicked()) continue;

        delay(1500);
        Serial.printf("Interrupt %d\n", buttonLongPressWatch.pin);
        
        if(systemButtons[buttonLongPressWatch.pin]->isPressed()){
            buttonLongPressWatch.func();
            Serial.println("DONE!");
        }
        buttonLongPressWatch = {0xFF, {}};
    }
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
    for(uint8_t i = 0; i < TOTAL_BUTTONS; i++){
        uint8_t ISRcount = ISREvents[i].size();
        for(uint8_t j = 0; j < ISRcount; j++){
            ISREvents[i][j].func = {};
        }
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

bool DebounceButton::addInterrupt(uint8_t buttonIndex, std::function<void(void)> func){
    return addInterrupt(buttonIndex, func, CLICK);
}

bool DebounceButton::addInterrupt(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode){
    if(buttonIndex > TOTAL_BUTTONS) Utilities::debug("Button %d is over the number of buttons (max. %d)!\n", buttonIndex, TOTAL_BUTTONS);
    for(uint8_t i = 0; i < ISREvents[buttonIndex].size(); i++){
        if(ISREvents[buttonIndex][i].functionalEvent == mode){
            Utilities::debug("Button %d is already in use with mode %d\n", buttonIndex, mode);
            return false;
        }
    }
    ISREvents[buttonIndex].push_back({func, mode});
    return true;
}

bool DebounceButton::removeInterrupt(uint8_t buttonIndex){
    ISREvents[buttonIndex].clear(); //Erases everything in this pin.
    return true;
}

DebounceButton::DebounceButton(uint8_t chipPin){
    pin = chipPin;
    pinMode(chipPin, INPUT);
}

bool DebounceButton::updateState(){
    bool currentPinState = digitalRead(pin);
    uint32_t currentTime = millis();
    Serial.printf("Curr:%d  Last:%d  Curr:%d Last:%d -> ", currentPinState, lastState, currentTime, lastTimePressed);
    if(currentPinState != lastState && (currentTime - lastTimePressed) > DEFAULT_DEBOUNCE_TIME){
        lastTimePressed = currentTime;
        bool ret = false;
        if(lastState == HIGH && currentPinState == LOW){
            buttonIsPressed = false;
            Serial.println("RELEASED");
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
            Serial.println("CLICK");
            ret = true;
        }
        lastState = currentPinState;
        return ret;
    }
    return false;
}

bool DebounceButton::clicked(){
    // If the state hasn't been updated, then it returns false so that the code that comes
    // after this, doesn't execute.
    return updateState() && buttonIsPressed;
}

bool DebounceButton::isPressed(){
    updateState();
    return buttonIsPressed;
}

bool DebounceButton::released(){
    return updateState() && !buttonIsPressed;
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

void DebounceButton::saveAndRemoveButtons(){
    for(uint8_t i = 0; i < 4; i++){
        previousISREvents[i] = ISREvents[i];
        removeInterrupt(i);
    }
}

void DebounceButton::undoRemoveButtons(){
    for(uint8_t i = 0; i < 4; i++){
        removeInterrupt(i);
        for(uint8_t j = 0; j < previousISREvents[i].size(); j++){
            ButtonFunction f = previousISREvents[i][j];
            addInterrupt(i, f.func, f.functionalEvent);
        }
    }
}