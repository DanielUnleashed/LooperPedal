#include "Metronome.h"

Metronome::Metronome(uint8_t tU, uint16_t t, uint8_t totalB, uint8_t beatUn){
    tempoUnit = tU;
    tempo = t;
    totalBeats = totalB;
    beatUnit = beatUn;
}

void Metronome::start(){
    digitalWrite(csPin, LOW);
    pinMode(csPin, OUTPUT);
    AuxSPI::begin();
    AuxSPI::sendToLEDs(csPin, 0);
    
    resume();
}

void Metronome::resume(){
    double timePerTempoUnit = 60000.0/tempo; //Working on ms

    if(totalBeats > 3 && totalBeats%3==0) isTerniary = true;

    timePerBeatUnit = timePerTempoUnit*tempoUnit/beatUnit;
    timePerBar = timePerBeatUnit*totalBeats;

    timePerBeatUnit *= isTerniary ? 3:1;
    
    //Serial.printf("tb: %d, tB: %d, rf:%d\n", timePerBeatUnit, timePerBar, isTerniary*3);

    startTime = millis();

    isPaused = false;
}

void Metronome::pause(){
    isPaused = true;
}

void Metronome::update(){
    if(isPaused) return;

    uint32_t currentTime = (millis()-startTime) % timePerBar;
    double tempoProgressDouble = ((double)currentTime)/timePerBeatUnit;

    uint8_t currentBeat = tempoProgressDouble;
    uint8_t nextBeat = currentBeat+1;
    nextBeat %= totalBeats/(isTerniary ? 3:1);

    currentBeat = convertToCompassType(currentBeat);
    nextBeat = convertToCompassType(nextBeat);
    uint8_t trans = getTransition(currentBeat, nextBeat, animationTiming(tempoProgressDouble));
    lightLEDs(trans);

    if(currentBeat == 0 && beginningFunc){
        beginningFunc();
        beginningFunc = {};
    }
    //Serial.printf("a: %d, sub:%d, trans:%d\n", currentBeat, subdivisionProgress, trans);
}

void IRAM_ATTR Metronome::doAtBeginningOfBeat(std::function<void(void)> func){
    beginningFunc = func;
}

/* Coverts the tempo progress decimals to [0,3] with desired timing.*/
uint8_t Metronome::animationTiming(double tempo){
    double t = tempo - ((uint8_t) tempo); // Only gets the decimals.
    double outF = 4.0*t*t;
    //Serial.printf("t:%f, y:%d\n",t, (uint8_t) outF);
    return (uint8_t) outF;
}

uint8_t Metronome::convertToCompassType(uint8_t tempoProgress){
    uint8_t realTotalBeats = totalBeats / (isTerniary ? 3:1);
    if(realTotalBeats == 2){
        return tempoProgress*3;
    }else if(realTotalBeats == 3){
        if(tempoProgress == 0) return 0;
        return tempoProgress+1;
    }else if(realTotalBeats == 4){
        return tempoProgress;
    }else{
        //Mixed beats
        return tempoProgress;
    }
}

uint8_t Metronome::getTransition(uint8_t i, uint8_t j, uint8_t t){
    if(t < 2){
        return transitions[5*i][t];
    }else{
        return transitions[4*i+j][t-2];
    }
}

void Metronome::lightLEDs(uint8_t ledCode){
    AuxSPI::sendToLEDs(csPin, 1<<ledCode);
}