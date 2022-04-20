#ifndef TASKBARBUTTON_h
#define TASKBARBUTTON_h

#include "Arduino.h"
#include <functional>

class TaskbarButton {
    public:
        TaskbarButton(String name, uint8_t position);        

        std::function<void(void)> pressedFunction = NULL;
        std::function<void(void)> released = NULL;
        std::function<void(void)> LEDtask = NULL;
        std::function<void(void)> draw = NULL;

        void addPressedFunction();
        void addReleasedFunction();
        void addLEDtask();
        void addDrawTask();
    protected: 
        bool isEnabled = false;
        String name;
        uint8_t position;
};

#endif