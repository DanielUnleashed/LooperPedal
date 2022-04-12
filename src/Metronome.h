#ifndef METRONOME_h
#define METRONOME_h

#include "Arduino.h"

class Metronome {
    public:
        Metronome(uint8_t tempoUnit, uint16_t tempo, uint8_t totalBeats, uint8_t beatUnit);

        void start();
        void update();

    private:
        uint8_t tempoUnit;
        uint16_t tempo;
        uint8_t totalBeats;
        uint8_t beatUnit;

        uint32_t startTime = 0;
        uint16_t timePerBeatUnit;
        uint16_t timePerBar;

        void lightLEDs(uint8_t ledCode);
};

#endif