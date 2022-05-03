#include "AudioPlayerWidget.h"
#include "UI/MenuManager.h"

static uint16_t color = 0x04B3;

AudioPlayerWidget::AudioPlayerWidget(uint8_t x, uint8_t y, uint8_t sizeX, uint8_t sizeY) : Widget("AudioPlayer", x, y, sizeX, sizeY, 2){

}

void AudioPlayerWidget::widgetDraw(){
    // Background
    drawRoundFilledRectangle(0,0,100,100,0x020D);

    // Play button
    drawCircle(20, 50, 14, color);
    //drawCircumference(20, 50, 14, 0x02AA);
    if(isSelected() && inWidgetSelection==0) 
        drawRoundRectangleByCenter(20,50,31,31,0xDD83);

    // Full play bar
    drawHLine(40,50, 50, 4, 0xA2AC);
    drawCircle(40,50,    2, 0xA2AC);
    drawCircle(90,50,    2, 0xA2AC);

    uint8_t playedPercentage = elapsedTime*100/totalTime;
    uint8_t playPoint = map(playedPercentage, 0, 100, 40, 90);

    // Elapsed play bar
    drawCircle(40,50,     1, 0xD8E9);
    drawHLine(40,50, playPoint-40, 2, 0xD8E9);
    //drawCircle(playPoint,50,     1, 0xD8E9); //This won't be seen
    // Dot at the end of the bar
    drawCircle(playPoint,50,6,0xFEFC);
    if(isSelected() && inWidgetSelection==1) 
        drawRoundRectangleByCenter(playPoint,50,15,15,0xDD83);

    String elT = Utilities::millisToString(elapsedTime);
    String elTotal = Utilities::millisToString(totalTime);
    drawText(40, 75, elT, TL_DATUM, 1, 0xFEFC);
    drawText(95, 75, elTotal, TR_DATUM, 1, 0xFEFC);
}

void AudioPlayerWidget::selectionFunctions(uint8_t selection){
    if(selection == 0){
        color = TFT_RED;
    }else if(selection == 1){
        Taskbar* t = MenuManager::getCurrentDisplay()->getTaskbar();
        if(t != NULL) t->saveAndRemoveButtons();
        DebounceButton::saveAndRemoveButtons();
        RotaryEncoder::saveAndRemoveInputs();

        t->addButton(0, "Start");
        t->addButton(1, "-5s");
        t->addButton(2, "+5s");
        t->addButton(3, "End");
        t->forceRedraw();

        DebounceButton::addInterrupt(0, [this]{
            elapsedTime = 0;
            this->redrawFromISR();
        });

        DebounceButton::addInterrupt(1, [this]{
            if(elapsedTime < 5000) elapsedTime = 0;
            else elapsedTime -= 5000;
            this->redrawFromISR();
        });

        DebounceButton::addInterrupt(2, [this]{
            if(totalTime - elapsedTime < 5000) elapsedTime = totalTime;
            else elapsedTime += 5000;
            this->redrawFromISR();
        });

        DebounceButton::addInterrupt(3, [this]{
            elapsedTime = totalTime;
            this->redrawFromISR();
        });

        RotaryEncoder::addInterrupt(0, [this](bool in){
            if(in){
                if(totalTime - elapsedTime < 1000) elapsedTime = totalTime;
                else elapsedTime+=1000;
            }else{
                if(elapsedTime < 1000) elapsedTime = 0;
                else elapsedTime-=1000;
            }
            this->redrawFromISR();
        });

        DebounceButton::addRotaryInterrupt(0, [this]{
            returnToPreviousInputs();
        });
    }

}