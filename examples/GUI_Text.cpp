#include "UI/MenuManager.h"
#include "UI/GUI/Taskbar.h"

void setup(){
  Serial.begin(115200);

  MenuManager::startMenuManager();
  Taskbar taskbar(0,3);
  taskbar.addButton("Chicken", 1);
  taskbar.addButton("Strips", 3);
  taskbar.draw();
}

void loop(){
  delay(1);
}