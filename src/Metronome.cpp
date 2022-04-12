#include "Metronome.h"

Metronome::Metronome(uint8_t tU, uint16_t t, uint8_t totalB, uint8_t beatUn){
    tempoUnit = tU;
    tempo = t;
    totalBeats = totalB;
    beatUnit = beatUn;

    double timePerTempoUnit = 60000.0/tempo; //Working on ms
    timePerBeatUnit = timePerTempoUnit*tempoUnit/beatUnit;
    timePerBar = timePerBeatUnit*totalBeats;
    Serial.printf("tU: %f, tb: %d, tB: %d\n", timePerTempoUnit, timePerBeatUnit, timePerBar);
}

void Metronome::start(){
    startTime = millis();
}

void Metronome::update(){
    uint32_t currentTime = (millis()-startTime) % timePerBar;
    uint8_t progress = currentTime/timePerBeatUnit;
}

void Metronome::lightLEDs(uint8_t ledCode){

}