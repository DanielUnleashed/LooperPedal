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
  delay(100);

  MenuManager::init();
  /*static Taskbar taskbar(0,3);
  taskbar.addButton("Chicken", 1);
  taskbar.addButton("Strips", 3);*/

  SplashScreen sp;

  for(;;){
    sp.draw();
    delay(100);
  }

/*  static Display disp1("Main");
  disp1.addItem(&sp);

  MenuManager::addDisplay(disp1);

  MenuManager::launch();*/

  /*AudioPlayer::begin();
  //AudioPlayer::addSDAudioFile("/tereza.wav");
  AudioPlayer::addRECAudioFile(0);*/
}

void loop() {}