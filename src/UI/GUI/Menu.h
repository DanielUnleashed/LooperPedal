#ifndef MENU_HEADER_h
#define MENU_HEADER_h

#include "DisplayItem.h"

class Menu : public DisplayItem{
    public:
    Menu(String menuName);

    String menuName;

    void draw() override;
};

#endif