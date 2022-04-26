#ifndef ROTARYENCODER_h
#define ROTARYENCODER_h

#include "Arduino.h"
#include "defines.h"
#include <functional>

#include "utils/Utilities.h"
#include "DebounceButton.h"

#define DEFAULT_ROTARY_DEBOUNCE 150

class RotaryEncoder{
    public:
        const int8_t ROTATION_DIRECTION[16] = {
            0,  -1,   1,  0,
            1,   0,   0, -1,
           -1,   0,   0,  1,
            0,   1,  -1,  0};

        RotaryEncoder(uint8_t chA, uint8_t chB);
        void addButton(uint8_t buttonPin);

        bool clicked();
        bool clicked(uint8_t timesPressed);
        bool doubleClicked();
        bool hasIncreased();

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
        volatile int8_t increment = 0;
        uint8_t chA, chB; //Pins

        DebounceButton* butt;
        bool hasButton = false;

        bool updateState();

        template <int interrupt>
        static void IRAM_ATTR ISR_ROTARY();
        template <int interrupt>
        static void IRAM_ATTR ISR_BUTTON();
        
};

#endif