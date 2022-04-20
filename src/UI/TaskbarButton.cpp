#include "TaskbarButton.h"

TaskbarButton::TaskbarButton(String n, uint8_t pos){
    name = n;
    position = pos;
}

void TaskbarButton::addPressedFunction(std::function<void(void)> func){
    pressedFunction = func;
}

