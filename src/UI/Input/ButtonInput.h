#ifndef BUTTONINPUT_h
#define BUTTONINPUT_h

#include "Arduino.h"
#include "defines.h"
#include <functional>
#include <vector>

#include "utils/Utilities.h"

#define DEFAULT_DEBOUNCE_TIME 40 //ms
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

class ButtonInput{
    public:
        static const uint8_t CLICK = 0;
        static const uint8_t LONG_PRESS = 1;

        ButtonInput();

        uint8_t pin;

        bool clicked();
        bool isPressed();
        bool released(); 
        bool clicked(uint8_t timesPressed);
        bool doubleClicked();
        static bool twoButtonsClicked(ButtonInput* otherButton);

        static void startButtons();

        bool addButtonFunction(std::function<void(void)> func, uint8_t mode);
        void saveAndRemoveButton();
        void undoRemoveButton();
        void clearEvents();

        static void clearLongPressButton();

    protected:
        volatile uint32_t lastTimePressed = 0;
        volatile uint32_t doubleClickedTime;
        volatile bool lastState = LOW;

        volatile bool buttonIsPressed = false;
        volatile uint8_t repeatedPressesCount = 0;

        // This stores all the functions of the buttons. Buttons may have several functions attached depending
        // of the input.
        std::vector<ButtonFunction> eventFunction;
        std::vector<ButtonFunction> previousEventFunction;

        bool updateState();

        static ButtonInput* buttonLongPressWatch;
        // Runs the event functions depending on the type of input (short, long press...)
        void eventEvaluation(int buttonIndex);

        virtual bool fetchInput() = 0;

    private:
        static TaskHandle_t buttonTaskHandle;
        static void longPressTimeTask(void* funcParams);

};

#endif