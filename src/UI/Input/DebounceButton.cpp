#include "DebounceButton.h"

DebounceButton* DebounceButton::systemButtons[TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS];
IRAM_ATTR std::vector<ButtonFunction> DebounceButton::ISREvents[TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS];
IRAM_ATTR std::vector<ButtonFunction> DebounceButton::previousISREvents[TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS];

ButtonEvent DebounceButton::buttonLongPressWatch = {0xFF, {}};

TaskHandle_t DebounceButton::buttonTaskHandle = NULL;

// Maybe implement with a loop with https://stackoverflow.com/questions/11081573/passing-a-variable-as-a-template-argument
void DebounceButton::init(){
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

    attachInterrupt(systemButtons[6]->pin, &ISR_BUTTON<6>, CHANGE);

    xTaskCreatePinnedToCore(longPressTimeTask, "WidgetEvents", 10000, NULL, 5, &buttonTaskHandle, 0);
}

template <int interrupt>
void IRAM_ATTR DebounceButton::ISR_BUTTON(){
    std::vector<ButtonFunction> eventList = ISREvents[interrupt];
    bool input = systemButtons[interrupt]->clicked();
    for(uint8_t i = 0; i < eventList.size(); i++){
        ButtonFunction ev = eventList[i];
        //Continue if there is no function.
        if(!ev.func) continue; 

        //Switch depending on the type of input assigned to the function.
        if(ev.functionalEvent == CLICK){
            if(input) ev.func();
        }else if(ev.functionalEvent == LONG_PRESS) {
            //Continue if a long press is being processed or if the button is released.
            if(interrupt == buttonLongPressWatch.pin || !input) continue;

            buttonLongPressWatch = {interrupt, ev.func};
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(buttonTaskHandle, &xHigherPriorityTaskWoken); 
        }
    }
}

void DebounceButton::longPressTimeTask(void* funcParams){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        delay(1000);
    
        if(systemButtons[buttonLongPressWatch.pin]->isPressed()){
            Serial.println("long!");
            buttonLongPressWatch.func();
        }
        buttonLongPressWatch.pin = 0xFF;
        buttonLongPressWatch.func = {};
    }
}

bool DebounceButton::updateState(){
    bool currentPinState = digitalRead(pin);
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
    for(uint8_t i = 0; i < TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS; i++){
        previousISREvents[i] = ISREvents[i];
        removeInterrupt(i);
    }
}

void DebounceButton::undoRemoveButtons(){
    for(uint8_t i = 0; i < TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS; i++){
        removeInterrupt(i);
        for(uint8_t j = 0; j < previousISREvents[i].size(); j++){
            ButtonFunction f = previousISREvents[i][j];
            addInterrupt(i, f.func, f.functionalEvent);
        }
    }
}

bool DebounceButton::addInterrupt(uint8_t buttonIndex, std::function<void(void)> func){
    return addInterrupt(buttonIndex, func, CLICK);
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
    for(uint8_t i = 0; i < ISREvents[buttonIndex].size(); i++){
        if(ISREvents[buttonIndex][i].functionalEvent == mode){
            Utilities::debug("Button %d is already in use with mode %d\n", buttonIndex, mode);
            return false;
        }
    }
    ISREvents[buttonIndex].push_back({func, mode});
    return true;
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
        ISREvents[i].clear();
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

bool DebounceButton::removeInterrupt(uint8_t buttonIndex){
    ISREvents[buttonIndex].clear(); //Erases everything in this pin.
    return true;
}

DebounceButton::DebounceButton(uint8_t chipPin){
    pin = chipPin;
    pinMode(chipPin, INPUT);
}