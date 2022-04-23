#include "AudioPlayer.h"
#include "UI/MenuManager.h"
#include "UI/GUI/Taskbar.h"

void setup() {
  Serial.begin(115200);
  Utilities::debug("\n*********************************************\n");
  Utilities::debug("    LOOPER PEDAL v0.1, by DanielUnleashed\n");
  Utilities::debug("*********************************************\n");
  delay(100);

  MenuManager::startMenuManager();
  Taskbar taskbar(0,3);
  taskbar.addButton("Chicken", 1);
  taskbar.addButton("Strips", 3);
  taskbar.draw();

  AudioPlayer::begin();
  AudioPlayer::addRECAudioFile(0);
}

void loop() {}