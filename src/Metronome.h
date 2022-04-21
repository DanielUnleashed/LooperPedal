#ifndef METRONOME_h
#define METRONOME_h

#include "Arduino.h"
#include "defines.h"
#include "utils/AuxSPI.h"

//TODO: Add wake-up method or waitXMillis till next animation method.

class Metronome {
    public:
        Metronome(uint8_t tempoUnit, uint16_t tempo, uint8_t totalBeats, uint8_t beatUnit);

        void start();
        void update();

    private:
        const uint8_t transitions[16][2] = {{0,0}, {1,3}, {1,5}, {1,7},
                                            {3,1}, {2,2}, {3,5}, {3,7},
                                            {5,1}, {5,3}, {4,4}, {5,7},
                                            {7,1}, {7,3}, {7,5}, {6,6}};

        uint8_t csPin = CS_METRONOME;

        uint8_t tempoUnit;
        uint16_t tempo;
        uint8_t totalBeats;
        uint8_t beatUnit;

        uint32_t startTime = 0;
        uint16_t timePerBeatUnit;
        uint16_t timePerBar;
        uint8_t isTerniary = false;

        uint8_t animationTiming(double subd);
        uint8_t getTransition(uint8_t i, uint8_t j, uint8_t t);
        uint8_t convertToCompassType(uint8_t tempoProgress);
        void lightLEDs(uint8_t ledCode);
};

#endif