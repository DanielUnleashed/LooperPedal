#include "Display.h"
#include "UI/MenuManager.h"

Display::Display(String n){
    name = n;
}

void Display::drawDisplay(TFT_eSprite &canvas){
    Widget::startDraw(canvas);
    for(DisplayItem* it : itemList)
        if(it -> needsToRedraw()) it -> render(canvas);
    Widget::finalDraw(canvas);
}

void Display::forceDraw(){
    for(DisplayItem* it : itemList)
        it -> forceRedraw();
    MenuManager::wakeUpDrawTask();
}

void Display::addItem(DisplayItem *item){
    itemList.push_back(item);
    if(item->itemName.equals("Widget")){
        Widget::addWidget((Widget*) item);
        Widget::sortDisplayedWidgetsList();
    }
    if(MenuManager::isLaunched){
        //item -> forceRedraw(); Not necesary!
        MenuManager::wakeUpDrawTask();
    }
}

void Display::removeItem(DisplayItem *item){
    for(uint8_t i = 0; i < itemList.size(); i++){
        if(item == itemList[i]){
            itemList.erase(itemList.begin()+i);
            return;
        }
    }
}

void Display::launchDisplay(){
    Widget::clearWidgets();
    for(DisplayItem *it : itemList){
        it -> attachEvents();
        if(it->itemName.equals("Widget")){
            Widget::addWidget((Widget*)it);
        }
    }
    Widget::sortDisplayedWidgetsList();
}

bool Display::hasTaskbar(){
    return taskbar != NULL;
}

void Display::addTaskbar(){
    taskbar = new Taskbar();
    itemList.push_back(taskbar);
}

Taskbar* Display::getTaskbar(){
    if(taskbar == NULL) Serial.println("Taskbar is null!\n");
    return taskbar;
}

DisplayItem* Display::getDisplayItem(uint8_t index){
    return itemList[index];
}