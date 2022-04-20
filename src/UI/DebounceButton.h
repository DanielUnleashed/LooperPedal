#ifndef DEBOUNCE_BUTTON_h
#define DEBOUNCE_BUTTON_h

#include "Arduino.h"
#include "defines.h"
#include <functional>

#define DEFAULT_DEBOUNCE_TIME 250 //ms
#define DEFAULT_DOUBLE_CLICK_TIME 800 //ms

class DebounceButton{
    public:
        DebounceButton(uint8_t chipPin);

        bool clicked();
        bool clicked(uint8_t timesPressed);
        bool doubleClicked();

        static DebounceButton* systemButtons[TOTAL_BUTTONS];
        static std::function<void(void)> ISREvents[TOTAL_BUTTONS];

        static void init();
        static void addInterrupt(uint8_t buttonIndex, std::function<void(void)> func);
        static void removeInterrupt(uint8_t buttonIndex);

    private:
        volatile uint32_t lastTimePressed = 0;
        volatile uint32_t doubleClickedTime;
        volatile bool lastState = LOW;
        uint8_t pin;

        volatile bool buttonIsPressed = false;
        volatile uint8_t repeatedPressesCount = 0;

        bool updateState();

        template <int interrupt>
        static void IRAM_ATTR ISR_BUTTON();
};

#endif