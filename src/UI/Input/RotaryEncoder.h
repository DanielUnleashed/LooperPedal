#ifndef ROTARYENCODER_h
#define ROTARYENCODER_h

#include "Arduino.h"
#include "defines.h"
#include <functional>

#include "utils/Utilities.h"
#include "DebounceButton.h"

class RotaryEncoder{
    public:
        RotaryEncoder(uint8_t chA, uint8_t chB);
        void addButton(uint8_t buttonPin);

        bool clicked();
        bool clicked(uint8_t timesPressed);
        bool doubleClicked();

        static RotaryEncoder* systemEncoders[TOTAL_ROTARY_ENCODERS];
        static DebounceButton* systemButtons[TOTAL_ROTARY_BUTTONS];
        static IRAM_ATTR std::function<void(bool)> ISREvents[TOTAL_ROTARY_ENCODERS];
        static IRAM_ATTR std::function<void(void)> ISRButtonEvents[TOTAL_ROTARY_BUTTONS];

        static void init();
        static bool addInterrupt(uint8_t rotatoryIndex, std::function<void(bool incr)> func);
        static bool addButtonInterrupt(uint8_t buttonIndex, std::function<void(void)> func);
        static bool removeInterrupt(uint8_t rotatoryIndex);
        static bool removeButtonInterrupt(uint8_t buttonIndex);

    private:
        volatile uint32_t lastTimeChange = 0;
        volatile uint8_t lastState = 0;
        uint8_t chA, chB;

        DebounceButton* butt;
        bool hasButton = false;

        bool updateState();
        bool hasIncreased();

        template <int interrupt>
        static void IRAM_ATTR ISR_ROTARY();
        template <int interrupt>
        static void IRAM_ATTR ISR_BUTTON();
        
};

#endif