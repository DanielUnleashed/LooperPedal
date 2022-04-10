#ifndef DEBOUNCE_BUTTON_h
#define DEBOUNCE_BUTTON_h

#include "Arduino.h"

#define DEFAULT_DEBOUNCE_TIME 250 //ms
#define DEFAULT_DOUBLE_CLICK_TIME 800 //ms

class DebounceButton{
    public:
        DebounceButton(uint8_t chipPin);

        bool buttonClicked();
        bool buttonClicked(uint8_t timesPressed);
        bool buttonDoubleClicked();
    private:
        uint32_t lastTimePressed;
        uint32_t doubleClickedTime;
        bool lastState = LOW;
        uint8_t pin;

        bool buttonIsPressed = false;
        uint8_t repeatedPressesCount = 0;

        bool updateState();
};

#endif