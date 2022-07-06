#include "RotaryEncoder.h"

RotaryEncoder* RotaryEncoder::systemEncoders[TOTAL_ROTARY_ENCODERS];
std::function<void(bool)> IRAM_ATTR RotaryEncoder::ISREvents[TOTAL_ROTARY_ENCODERS];
std::function<void(bool)> IRAM_ATTR RotaryEncoder::previousISREvents[TOTAL_ROTARY_ENCODERS];

// Maybe implement with a loop with https://stackoverflow.com/questions/11081573/passing-a-variable-as-a-template-argument
void RotaryEncoder::init(){
    for(uint8_t i = 0; i < TOTAL_ROTARY_ENCODERS; i++){
        systemEncoders[i] = new RotaryEncoder(ROTARY_ENCODERS[i*2], ROTARY_ENCODERS[i*2+1]);
    }

    // Rotary encoders interrupts
    attachInterrupt(systemEncoders[0]->chA, &ISR_ROTARY<0>, CHANGE);
    attachInterrupt(systemEncoders[0]->chB, &ISR_ROTARY<0>, CHANGE);

    // Rotary buttons
    //attachInterrupt(systemButtons[0]->pin, &ISR_BUTTON<0>, CHANGE);
}

template <int interrupt>
void IRAM_ATTR RotaryEncoder::ISR_ROTARY(){
    // Will check if the ISR has been added and the encoder input.
    if(ISREvents[interrupt] && systemEncoders[interrupt] -> updateState())
        ISREvents[interrupt](systemEncoders[interrupt] -> hasIncreased()); 
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

bool RotaryEncoder::removeInterrupt(uint8_t rotatoryIndex){
    ISREvents[rotatoryIndex] = {};
    return !ISREvents[rotatoryIndex];
}

void RotaryEncoder::saveAndRemoveInputs(){
    for(uint8_t i = 0; i < TOTAL_ROTARY_ENCODERS; i++){
        previousISREvents[i] = ISREvents[i];
        removeInterrupt(i);
    }
}

void RotaryEncoder::undoRemoveInputs(){
    for(uint8_t i = 0; i < TOTAL_ROTARY_ENCODERS; i++){
        removeInterrupt(i);
        addInterrupt(i, previousISREvents[i]);
    }
}

bool RotaryEncoder::clearAll(){
    for(std::function<void(bool)> &f : ISREvents) f = {};
    return true;
}

RotaryEncoder::RotaryEncoder(uint8_t a, uint8_t b){
    chA = a;
    chB = b;
    pinMode(chA, INPUT);
    pinMode(chB, INPUT);
    lastState = digitalRead(chA) | (digitalRead(chB) << 1);
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