#ifndef ANALOGBUTTON_h
#define ANALOGBUTTON_h

#include "ButtonInput.h"

class AnalogButton : public ButtonInput{
    public:
        static const uint8_t SAMPLING_COUNT = 5;
        static const uint8_t THRESHOLD = 5;

        AnalogButton();

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
        static bool addInterrupt(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode);
        static bool addMultipleInterrupt(uint8_t* buttonIndexes, std::function<void(void)> func);
        static bool clearMultipleInterrupt(uint8_t* buttonIndexes);
        static bool clearAll();
        static bool removeInterrupt(uint8_t buttonIndex);

        static void saveAndRemoveButtons();
        static void undoRemoveButtons();

    private:
        static AnalogButton* systemButtons[TOTAL_ANALOG_BUTTONS];
        static uint16_t* readValues;

        bool currentInput = false;
        bool fetchInput() override;

        static uint16_t valueRead;
        static void analogPollingFunc(void* funcParams);

        // Calculates the voltage received in the analog input pin when the set of buttons inside arrIn are 
        // pressed. 
        // @param arrIn. The array of buttons that are being pressed.
        // @param layer. The layer of combinations (layer 2 means that two buttons are being pressed).
        // @param count. Counts the number of points that are being created. This number will be used to
        // access readValues array.
        // @inputResistance. The resistance of the button. Is passed to be calculated in parallel with the new button. 
        static void calculateVoltageValues(String arrIn, uint8_t layer, uint8_t &count, double inputResistance);

        static bool compareVoltageValues(String arrIn, uint8_t layer, uint8_t &count, uint16_t readValue);
        static uint16_t combinatorial(uint16_t n, uint16_t k);
        static uint16_t factorial(uint16_t x);
};

#endif