#ifndef DEBOUNCE_BUTTON_h
#define DEBOUNCE_BUTTON_h

#include "Arduino.h"
#include "defines.h"
#include <functional>

#include "utils/Utilities.h"

#define DEFAULT_DEBOUNCE_TIME 200 //ms
#define DEFAULT_DOUBLE_CLICK_TIME 800 //ms

class DebounceButton{
    public:
        DebounceButton(uint8_t chipPin);
        uint8_t pin;

        bool clicked();
        bool clicked(uint8_t timesPressed);
        bool doubleClicked();
        static bool twoButtonsClicked(uint8_t otherButton);

        static DebounceButton* systemButtons[TOTAL_BUTTONS];
        static IRAM_ATTR std::function<void(void)> ISREvents[TOTAL_BUTTONS];

        static void init();
        static bool addInterrupt(uint8_t buttonIndex, std::function<void(void)> func);
        static bool removeInterrupt(uint8_t buttonIndex);

    private:
        volatile uint32_t lastTimePressed = 0;
        volatile uint32_t doubleClickedTime;
        volatile bool lastState = LOW;

        volatile bool buttonIsPressed = false;
        volatile uint8_t repeatedPressesCount = 0;

        bool updateState();

        template <int interrupt>
        static void IRAM_ATTR ISR_BUTTON();
};

#endif