#ifndef TASKBAR_h
#define TASKBAR_h

#include "DisplayItem.h"

struct TaskbarButton{
    bool isEnabled = false;
    String tagName = "";
};

class Taskbar : public DisplayItem{

    public:
    Taskbar();
    void draw() override;

    bool addButton(uint8_t index, String tagName);
    bool removeButton(uint8_t index);

    void saveAndRemoveButtons();
    void undoRemoveButtons();

    private: 
    TaskbarButton buttons[4];
    TaskbarButton previousButtons[4];

};

#endif