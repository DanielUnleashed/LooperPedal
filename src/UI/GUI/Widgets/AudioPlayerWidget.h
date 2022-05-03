#ifndef AUDIOPLAYER_WIDGET_h
#define AUDIOPLAYER_WIDGET_h

#include "UI/GUI/Widgets/Widget.h"
#include "AudioPlayer.h"

class AudioPlayerWidget : public Widget{

  public:
  AudioPlayerWidget(uint8_t x, uint8_t y, uint8_t sizeX, uint8_t sizeY);

  void widgetDraw() override;
  void selectionFunctions(uint8_t selection) override;

  private:
  uint32_t elapsedTime = 4000;
  uint32_t totalTime = 100000;

};

#endif