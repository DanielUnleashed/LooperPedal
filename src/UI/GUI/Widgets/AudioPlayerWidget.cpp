#include "AudioPlayerWidget.h"

AudioPlayerWidget::AudioPlayerWidget(uint8_t x, uint8_t y, uint8_t sizeX, uint8_t sizeY) : Widget("AudioPlayerWidget", x, y, sizeX, sizeY, 2){

}

void AudioPlayerWidget::widgetDraw(){
    // Background
    drawRoundFilledRectangle(0,0,100,100,0x020D);

    // Play button
    drawCircle(20, 50, 14, 0x04B3);
    drawCircumference(20, 50, 14, 0x02AA);
    if(isSelected() && inWidgetSelection==0) 
        drawRoundRectangleByCenter(20,50,31,31,0xDD83);

    // Full play bar
    drawHLine(40,50, 55, 4, 0xA2AC);
    drawCircle(40,50,    2, 0xA2AC);
    drawCircle(95,50,    2, 0xA2AC);

    // Elapsed play bar
    drawHLine(40, 50, 25, 2, 0xD8E9);
    drawCircle(40,50,     1, 0xD8E9);
    drawCircle(65,50,     1, 0xD8E9);
    // Dot at the end of the bar
    drawCircle(65,50,6,0xFEFC);
    if(isSelected() && inWidgetSelection==1) 
        drawRoundRectangleByCenter(65,50,15,15,0xDD83);

    drawText(40, 75, "0:30", TL_DATUM, 1, 0xFEFC);
    drawText(95, 75, "1:05", TR_DATUM, 1, 0xFEFC);
}