#ifndef TASKBARBUTTON_h
#define TASKBARBUTTON_h

#include "Arduino.h"
#include <functional>

class TaskbarButton {
    public:
        TaskbarButton(String name, uint8_t position);        

    protected: 
        bool isEnabled = false;
        String name;
        uint8_t position;
};

#endif