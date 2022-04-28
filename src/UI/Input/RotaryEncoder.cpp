#include "RotaryEncoder.h"

RotaryEncoder* RotaryEncoder::systemEncoders[TOTAL_ROTARY_ENCODERS];
DebounceButton* RotaryEncoder::systemButtons[TOTAL_ROTARY_BUTTONS];
std::function<void(bool)> IRAM_ATTR RotaryEncoder::ISREvents[TOTAL_ROTARY_ENCODERS];
std::function<void(void)> IRAM_ATTR RotaryEncoder::ISRButtonEvents[TOTAL_ROTARY_BUTTONS];

// Maybe implement with a loop with https://stackoverflow.com/questions/11081573/passing-a-variable-as-a-template-argument
void RotaryEncoder::init(){
    for(uint8_t i = 0; i < TOTAL_ROTARY_ENCODERS; i++){
        systemEncoders[i] = new RotaryEncoder(ROTARY_ENCODERS[i*2], ROTARY_ENCODERS[i*2+1]);
        if(ROTARY_BUTTONS[i] != 0xFF){
            systemEncoders[i] -> addButton(ROTARY_BUTTONS[i]);
            systemButtons[i] = systemEncoders[i] -> butt;
        }
    }

    // Rotary encoders interrupts
    attachInterrupt(systemEncoders[0]->chA, &ISR_ROTARY<0>, CHANGE);
    attachInterrupt(systemEncoders[0]->chB, &ISR_ROTARY<0>, CHANGE);

    // Rotary buttons
    attachInterrupt(systemButtons[0]->pin, &ISR_BUTTON<0>, CHANGE);
}

template <int interrupt>
void IRAM_ATTR RotaryEncoder::ISR_ROTARY(){
    // Will check if the ISR has been added and the encoder input.
    if(ISREvents[interrupt] && systemEncoders[interrupt] -> updateState())
        ISREvents[interrupt](systemEncoders[interrupt] -> hasIncreased()); 
}

template <int interrupt>
void IRAM_ATTR RotaryEncoder::ISR_BUTTON(){
    // Will check if the ISR has been added and 
    if(ISRButtonEvents[interrupt] && systemButtons[interrupt] -> clicked()) ISRButtonEvents[interrupt](); 
}

bool RotaryEncoder::addInterrupt(uint8_t rotatoryIndex, std::function<void(bool)> func){
    if(ISREvents[rotatoryIndex]){
        Utilities::debug("Rotatory encoder %d is already in use\n", rotatoryIndex);
        return false;
    }else{
        ISREvents[rotatoryIndex] = func;
        return true;
    }
}

bool RotaryEncoder::addButtonInterrupt(uint8_t buttonIndex, std::function<void(void)> func){
    if(ISRButtonEvents[buttonIndex]){
        Utilities::debug("Rotary button %d is already in use\n", buttonIndex);
        return false;
    }else{
        ISRButtonEvents[buttonIndex] = func;
        return true;
    }
}

bool RotaryEncoder::removeInterrupt(uint8_t rotatoryIndex){
    ISREvents[rotatoryIndex] = {};
    return !ISREvents[rotatoryIndex];
}

bool RotaryEncoder::removeButtonInterrupt(uint8_t buttonIndex){
    ISRButtonEvents[buttonIndex] = {};
    return !ISRButtonEvents[buttonIndex];
}

bool RotaryEncoder::clearAll(){
    for(std::function<void(bool)> &f : ISREvents) f = {};
    for(std::function<void(void)> &f : ISRButtonEvents) f = {};
    return true;
}

RotaryEncoder::RotaryEncoder(uint8_t a, uint8_t b){
    chA = a;
    chB = b;
    pinMode(chA, INPUT);
    pinMode(chB, INPUT);
    lastState = digitalRead(chA) | (digitalRead(chB) << 1);
}

void RotaryEncoder::addButton(uint8_t buttonPin){
    butt = new DebounceButton(buttonPin);
    hasButton = true;
}

bool RotaryEncoder::updateState(){
    uint8_t currentPinState = digitalRead(chA) | (digitalRead(chB) << 1);
    uint32_t currentTime = millis();
    if(currentPinState != lastState){
        if((currentTime - lastTimeChange) > DEFAULT_ROTARY_DEBOUNCE){
            lastTimeChange = currentTime;
            increment = ROTATION_DIRECTION[currentPinState | (lastState << 2)];
            return true;
        }
        lastState = currentPinState;
    }
    return false;
}

bool RotaryEncoder::hasIncreased(){
    return increment == 1;
}

bool RotaryEncoder::clicked(){
    // If the state hasn't been updated, then it returns false so that the code that comes
    // after this, doesn't execute.
    return hasButton && butt->clicked();
}

bool RotaryEncoder::clicked(uint8_t timesPressed){
    return hasButton && butt->clicked(timesPressed);
}

bool RotaryEncoder::doubleClicked(){
    return hasButton && butt->doubleClicked();
}