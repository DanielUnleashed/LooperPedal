#ifndef ROTARYENCODER_h
#define ROTARYENCODER_h

#include "Arduino.h"
#include "defines.h"
#include <functional>

#include "utils/Utilities.h"
#include "DebounceButton.h"

#define DEFAULT_ROTARY_DEBOUNCE 120

struct RotaryEncoderEvent{
    uint8_t pin;
    std::function<void(bool)> func;
};

class RotaryEncoder{
    public:
        const int8_t ROTATION_DIRECTION[16] = {
            0,  -1,   1,  0,
            1,   0,   0, -1,
           -1,   0,   0,  1,
            0,   1,  -1,  0};

        RotaryEncoder(uint8_t chA, uint8_t chB);

        bool hasIncreased();

        static RotaryEncoder* systemEncoders[TOTAL_ROTARY_ENCODERS];
        static IRAM_ATTR std::function<void(bool)> ISREvents[TOTAL_ROTARY_ENCODERS];
        static IRAM_ATTR std::function<void(bool)> previousISREvents[TOTAL_ROTARY_ENCODERS];

        static void init();
        static bool addInterrupt(uint8_t rotatoryIndex, std::function<void(bool incr)> func);
        static bool clearAll();
        static bool removeInterrupt(uint8_t rotatoryIndex);

        static void saveAndRemoveInputs();
        static void undoRemoveInputs();

    private:
        volatile uint32_t lastTimeChange = 0;
        volatile uint8_t lastState = 0;
        volatile int8_t increment = 0;
        uint8_t chA, chB; //Pins

        bool updateState();

        template <int interrupt>
        static void IRAM_ATTR ISR_ROTARY();
        
};

#endif