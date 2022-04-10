#ifndef DEBOUNCE_BUTTON_h
#define DEBOUNCE_BUTTON_h

#include "Arduino.h"

#define DEFAULT_DEBOUNCE_TIME 250 //ms
#define DEFAULT_DOUBLE_CLICK_TIME 800 //ms

class DebounceButton{
    public:
        DebounceButton(uint8_t chipPin);

        bool clicked();
        bool clicked(uint8_t timesPressed);
        bool doubleClicked();
    private:
        volatile uint32_t lastTimePressed = 0;
        volatile uint32_t doubleClickedTime;
        volatile bool lastState = LOW;
        uint8_t pin;

        volatile bool buttonIsPressed = false;
        volatile uint8_t repeatedPressesCount = 0;

        bool updateState();
};

#endif