#include "AnalogButton.h"
#include "UI/MenuManager.h"

AnalogButton* AnalogButton::systemButtons[TOTAL_ANALOG_BUTTONS];
uint16_t* AnalogButton::readValues = NULL;

uint16_t AnalogButton::valueRead = 0;

void AnalogButton::init(){
    static_assert(TOTAL_ANALOG_BUTTONS >= MAX_ANALOG_COMBINATIONS || TOTAL_ANALOG_BUTTONS==0, "TOTAL_ANALOG_BUTTONS must be greater than MAX_ANALOG_COMBINATIONS");

    startButtons();

    // The number of buttons is the sum of the combinatorials from k=1 to k=MAX_ANALOG_COMBINATIONS.
    // If MAX_ANALOG_COMBINATIONS == TOTAL_ANALOG_BUTTONS, then this sum equals 2^TOTAL_ANALOG_BUTTONS-1
    uint16_t combinationCount = 0;
    #if MAX_ANALOG_COMBINATIONS == TOTAL_ANALOG_BUTTONS 
        combinationCount = 1<<TOTAL_ANALOG_BUTTONS - 1; //Remove one because "no reading" is not needed to be saved.
    #else
        for(uint16_t m = 1; m <= MAX_ANALOG_COMBINATIONS; m++){
            combinationCount += combinatorial(TOTAL_ANALOG_BUTTONS, m);
        }
    #endif
    readValues = (uint16_t*)malloc(combinationCount*sizeof(uint16_t));

    pinMode(ANALOG_INPUT_PIN, INPUT);

    // Load in the upper part of the array the voltages with only one button. This will be the most normal
    // case: if pressed two buttons at the same time, first one input will be detected and then the next one.
    // That's human behaviour!
    for(uint8_t i = 0; i < TOTAL_ANALOG_BUTTONS; i++){
        systemButtons[i] = new AnalogButton();
        readValues[i] = INPUT_RESISTANCE/(ANALOG_RESISTANCES[i]+INPUT_RESISTANCE)*4095;
    }
    // And then load the combinations of buttons.
    uint8_t count = TOTAL_ANALOG_BUTTONS;
    for(uint8_t i = 0; i < TOTAL_ANALOG_BUTTONS; i++){
        calculateVoltageValues(String(i), 0, count, readValues[i]);
    }

    xTaskCreatePinnedToCore(analogPollingFunc, "AnalogPollingButton Task", 10000, NULL, 5, NULL, 0);
}

void AnalogButton::calculateVoltageValues(String arrIn, uint8_t layer, uint8_t &count, double inputResistance){
    for(int j = arrIn.substring(layer).toInt()+1; j < TOTAL_ANALOG_BUTTONS; j++){
        String comb = arrIn + j;
        double downRes = inputResistance*ANALOG_RESISTANCES[j]/(INPUT_RESISTANCE + ANALOG_RESISTANCES[j]);
        readValues[count++] = downRes/(INPUT_RESISTANCE+downRes)*4095;
        
        if(layer+1 < MAX_ANALOG_COMBINATIONS-1) calculateVoltageValues(comb, layer+1, count, downRes);
    }
}

void AnalogButton::analogPollingFunc(void* funcParams){
    static uint16_t lastValue = 0;
    
    for(;;){
        // Calculate the analog input value. The sample must be taken several times to not 
        // mix up the buttons readings.
        uint16_t av = 0;
        for(uint8_t i = 0; i < SAMPLING_COUNT; i++) av += analogRead(ANALOG_INPUT_PIN);
        valueRead = av/SAMPLING_COUNT;

        if(valueRead == lastValue){
            delay(25);
            continue;
        }

        lastValue = valueRead;
        bool buttonFound = false;
        // First, check the single button inputs.
        for(uint8_t i = 0; i < TOTAL_ANALOG_BUTTONS; i++){
            systemButtons[i]->currentInput = false; // Reset all inputs
            if((buttonFound = (readValues[i] == valueRead))){
                systemButtons[i]->valueRead = true;
                systemButtons[i]->eventEvaluation();
                break;
            }
        }
        
        // If no button has been found, then it must be a button combination. Try now to find them.
        if(!buttonFound){
            uint8_t count = TOTAL_ANALOG_BUTTONS;
            for(uint8_t i = 0; i < TOTAL_ANALOG_BUTTONS; i++){
                if((buttonFound = compareVoltageValues(String(i), 0, count, readValues[i]))) break;
            }
            if(!buttonFound) Serial.printf("Missed an input with value %d\n", valueRead);
        }
        delay(25);
    }
}

bool AnalogButton::compareVoltageValues(String arrIn, uint8_t layer, uint8_t &count, uint16_t readValue){
    for(int j = arrIn.substring(layer).toInt()+1; j < TOTAL_ANALOG_BUTTONS; j++){
        String comb = arrIn + j;
        if(readValue == readValues[count++]){
            for(uint8_t i = 0; i < comb.length()-1; i++){ // length-1 because the string ends in NULL (0).
                uint8_t but = arrIn[i] - 48; // Char to int conversion
                systemButtons[i]->valueRead = true;
                systemButtons[but]->eventEvaluation();
            }
            return true;
        }
        if(layer+1 < MAX_ANALOG_COMBINATIONS-1) return compareVoltageValues(comb, layer+1, count, readValue);
    }
    return false;
}

void AnalogButton::saveAndRemoveButtons(){
    ButtonInput::clearLongPressButton();
    for(uint8_t i = 0; i < TOTAL_ANALOG_BUTTONS; i++){
        systemButtons[i]->saveAndRemoveButton();
    }
}

void AnalogButton::undoRemoveButtons(){
    for(uint8_t i = 0; i < TOTAL_ANALOG_BUTTONS; i++){
        systemButtons[i]->undoRemoveButton();
    }
}

bool AnalogButton::addInterrupt(uint8_t buttonIndex, std::function<void(void)> func){
    return addInterrupt(buttonIndex, func, CLICK);
}

bool AnalogButton::addInterrupt(uint8_t buttonIndex, String tagName, std::function<void(void)> func){
    Display* d = MenuManager::getCurrentDisplay();
    if(d!=NULL){
        Taskbar* t = d->getTaskbar();
        if(t!=NULL) t->addButton(buttonIndex, tagName);
    }
    return addInterrupt(buttonIndex, func, CLICK);
}

bool AnalogButton::addInterrupt(uint8_t buttonIndex, String tagName, std::function<void(void)> func, uint8_t mode){
    Display* d = MenuManager::getCurrentDisplay();
    if(d!=NULL){
        Taskbar* t = d->getTaskbar();
        if(t!=NULL) t->addButton(buttonIndex, tagName);
    }
    return addInterrupt(buttonIndex, func, mode);
}

bool AnalogButton::addInterrupt(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode){
    if(buttonIndex > TOTAL_ANALOG_BUTTONS) Utilities::debug("%sButton %d is over the number of buttons!\n", 
        buttonIndex>TOTAL_ANALOG_BUTTONS?"Rotary":"",  
        buttonIndex>TOTAL_ANALOG_BUTTONS?buttonIndex-TOTAL_ANALOG_BUTTONS:buttonIndex);

    bool ok = systemButtons[buttonIndex]->addButtonFunction(func, mode);
    if(!ok) Serial.printf(" Button %d is already in use!", buttonIndex);
    return ok;
}

bool AnalogButton::addMultipleInterrupt(uint8_t* buttonIndexes, std::function<void(void)> func){
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

bool AnalogButton::clearAll(){
    for(uint8_t i = 0; i < TOTAL_ANALOG_BUTTONS; i++){
        systemButtons[i]->clearEvents();
    }
    return true;
}

bool AnalogButton::clearMultipleInterrupt(uint8_t* buttonIndexes){
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

AnalogButton::AnalogButton():ButtonInput(){}

bool AnalogButton::fetchInput(){
    return valueRead;
}

uint16_t AnalogButton::combinatorial(uint16_t n, uint16_t k){
    return factorial(n)/factorial(k)/factorial(n-k);
}

uint16_t AnalogButton::factorial(uint16_t x){
    uint16_t ans = 1;
    for(uint16_t i = 2; i <= x; i++) ans *= i;
    return ans;
}