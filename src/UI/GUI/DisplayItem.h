#ifndef DISPLAYITEM_h
#define DISPLAYITEM_h

#include "Arduino.h"
#include <functional>

#include "UI/Input/DebounceButton.h"
#include "UI/Input/RotaryEncoder.h"

#include <TFT_eSPI.h>

#include <vector>

// The FPS whomst value was used to design the animation speeds.
#define FPS_DESIGN 50.0

struct Point{
    uint16_t x, y;
};

class  DisplayItem {
    public:
    DisplayItem(String name); //Full screen item
    
    void render(TFT_eSprite&);

    static void startDisplayItems(uint16_t width, uint16_t height);

    String itemName;
    bool needsUpdate = true; // If it CURRENTLY needs update.
    bool needsToRedraw();
    void forceRedraw();

    void attachEvents();

    private:
    virtual void draw() = 0;

    protected:
    static uint16_t width, height;

    TFT_eSprite *canvas;

    //Events
    std::vector<ButtonEvent> buttonEvents;
    std::vector<RotaryEncoderEvent> rotaryEvents;
    std::vector<ButtonEvent> rotaryButtonEvents;

    void addButtonEvent(uint8_t buttonIndex, std::function<void(void)> func);
    void addButtonEvent(uint8_t buttonIndex, std::function<void(void)> func, String name);
    void addButtonEvent(uint8_t buttonIndex, std::function<void(void)> func, String name, uint8_t mode);
    void addRotaryEvent(uint8_t rotatoryIndex, std::function<void(bool incr)> func);
    void addRotaryButtonEvent(uint8_t buttonIndex, std::function<void(void)> func);
    void addRotaryButtonEvent(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode);

    //Graphics
    void startAnimation();
    void redraw();
    void redrawFromISR();
    void endAnimation();
    uint32_t startAnimationTime;

    uint32_t getTickTime();

};

#endif