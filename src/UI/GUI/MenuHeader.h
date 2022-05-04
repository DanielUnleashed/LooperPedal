#ifndef MENU_HEADER_h
#define MENU_HEADER_h

#include "DisplayItem.h"

class MenuHeader : public DisplayItem{

    public:
    MenuHeader(String name);

    void draw() override;

};

#endif