#include "UI/MenuManager.h"
#include "UI/GUI/Taskbar.h"

Taskbar taskbar(1,1);

void setup(){
  Serial.begin(115200);

  MenuManager::startMenuManager();
  taskbar.draw();
}

void loop(){

}