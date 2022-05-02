#ifndef DEBOUNCE_BUTTON_h
#define DEBOUNCE_BUTTON_h

#include "Arduino.h"
#include "defines.h"
#include <functional>
#include <vector>

#include "utils/Utilities.h"

#define DEFAULT_DEBOUNCE_TIME 200 //ms
#define DEFAULT_DOUBLE_CLICK_TIME 800 //ms

struct ButtonEvent{
    uint8_t pin;
    std::function<void(void)> func;
    String name;
    uint8_t functionalEvent;
};

struct ButtonFunction{
    std::function<void(void)> func;
    uint8_t functionalEvent;
};

class DebounceButton{
    public:
        static const uint8_t CLICK = 0;
        static const uint8_t LONG_PRESS = 1; 

        DebounceButton(uint8_t chipPin);
        uint8_t pin;

        bool clicked();
        bool isPressed();
        bool released(); 
        bool clicked(uint8_t timesPressed);
        bool doubleClicked();
        static bool twoButtonsClicked(uint8_t otherButton);

        static void init();
        // Adds an interrupt function to buttonIndex that will trigger when the button is clicked.
        // @returns False if the interrupt could not be added.
        static bool addInterrupt(uint8_t buttonIndex, std::function<void(void)> func);
        // Adds an interrupt function to buttonIndex that will trigger depending on the mode.
        // @param mode CLICK, LONG_PRESS...
        // @returns False if the interrupt could not be added.
        static bool addInterrupt(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode);
        static bool addMultipleInterrupt(uint8_t* buttonIndexes, std::function<void(void)> func);
        static bool clearMultipleInterrupt(uint8_t* buttonIndexes);
        static bool clearAll();
        static bool removeInterrupt(uint8_t buttonIndex);

        static void saveAndRemoveButtons();
        static void undoRemoveButtons();

    private:
        volatile uint32_t lastTimePressed = 0;
        volatile uint32_t doubleClickedTime;
        volatile bool lastState = LOW;

        volatile bool buttonIsPressed = false;
        volatile uint8_t repeatedPressesCount = 0;

        static DebounceButton* systemButtons[TOTAL_BUTTONS];
        // This stores all the functions of the buttons. Buttons may have several functions attached depending
        // of the input.
        static std::vector<ButtonFunction> ISREvents[TOTAL_BUTTONS];
        static std::vector<ButtonFunction> previousISREvents[TOTAL_BUTTONS];
        bool updateState();

        static ButtonEvent buttonLongPressWatch;

        template <int interrupt>
        static void IRAM_ATTR ISR_BUTTON();

        static TaskHandle_t buttonTaskHandle;
        static void longPressTimeTask(void* funcParams);
};

#endif