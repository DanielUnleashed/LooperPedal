#ifndef TASKBAR_h
#define TASKBAR_h

#include "DisplayItem.h"

typedef struct TaskbarButton{
    bool isEnabled = false;
    String tagName;
    uint8_t position;
};


class Taskbar : public DisplayItem{

    const uint8_t sX = 4;
    const uint8_t sY = 1;

    public:
    Taskbar(uint8_t tileX, uint8_t tileY);
    TaskbarButton buttons[4];

    void draw() override;
};

#endif