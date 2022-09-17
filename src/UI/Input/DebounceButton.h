#ifndef DEBOUNCE_BUTTON_h
#define DEBOUNCE_BUTTON_h

#include "ButtonInput.h"

class DebounceButton : public ButtonInput{
    public:
        DebounceButton(uint8_t chipPin);

        static void init();

        // Adds an interrupt function to buttonIndex that will trigger when the button is clicked.
        // @returns False if the interrupt could not be added.
        static bool addInterrupt(uint8_t buttonIndex, std::function<void(void)> func);

        // Adds an interrupt function to buttonIndex that will trigger when the button is clicked.
        // Also attaches a tag to the Taskbar if available.
        // @returns False if the interrupt could not be added.
        static bool addInterrupt(uint8_t buttonIndex, String tagName, std::function<void(void)> func);
        static bool addInterrupt(uint8_t buttonIndex, String tagName, std::function<void(void)> func, uint8_t mode);
        
        // Adds an interrupt function to buttonIndex that will trigger depending on the mode.
        // @param mode CLICK, LONG_PRESS...
        // @returns False if the interrupt could not be added.
        static bool addRotaryInterrupt(uint8_t buttonIndex, std::function<void(void)> func);
        static bool addRotaryInterrupt(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode);
        static bool addInterrupt(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode);
        static bool addMultipleInterrupt(uint8_t* buttonIndexes, std::function<void(void)> func);

        static bool clearMultipleInterrupt(uint8_t* buttonIndexes);
        static bool clearAll();
        static bool removeInterrupt(uint8_t buttonIndex);

        static void saveAndRemoveButtons();
        static void undoRemoveButtons();

        static void saveAndRemoveScreenButtons();
        static void undoRemoveScreenButtons();

    private:
        static DebounceButton* systemButtons[TOTAL_BUTTONS+TOTAL_ROTARY_BUTTONS];

        bool fetchInput() override;

        // Envelope for the static class needed to implement an ISR.
        template <int interrupt>
        static void IRAM_ATTR ISR_BUTTON();
};

#endif