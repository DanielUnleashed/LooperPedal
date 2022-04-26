#include "AudioPlayer.h"
#include "UI/MenuManager.h"
#include "UI/GUI/Taskbar.h"
#include "UI/GUI/SplashScreen.h"
#include "UI/Display.h"

void setup() {
  Serial.begin(115200);
  Utilities::debug("\n*********************************************\n");
  Utilities::debug("    LOOPER PEDAL v0.1, by DanielUnleashed\n");
  Utilities::debug("*********************************************\n");
  delay(50);

  MenuManager::init();

  static SplashScreen sp;
  static Display introDisplay("Intro");

  static Taskbar taskbar(0,3);
  static Display mainDisplay("Main");

  introDisplay.addItem(&sp);

  taskbar.addButton("Chicken", 1);
  taskbar.addButton("Strips", 3);

  mainDisplay.addItem(&taskbar);

  MenuManager::addDisplay(introDisplay);
  MenuManager::addDisplay(mainDisplay);
  MenuManager::launch();

  /*AudioPlayer::begin();
  //AudioPlayer::addSDAudioFile("/tereza.wav");
  AudioPlayer::addRECAudioFile(0);*/
}

void loop() {}