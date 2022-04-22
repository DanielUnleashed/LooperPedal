#ifndef defines_h
#define defines_h

//#define LAUNCH_CONSOLE

//#define USE_BOTH_ADC_CHANNELS

// Branch inside AudioPlayer.cpp
#define PASS_AUDIO_INPUT_DURING_RECORDING

#define PLAY_FREQUENCY 16000
#define BIT_RES 16

// If more were to be added, add to the init() method inside DebounceButton.cpp
// another call to attachInterrupt with the correspondant number.
#define TOTAL_BUTTONS 4
static const uint8_t PUSH_BUTTON[TOTAL_BUTTONS] = {34,35,36,39};

#define CS_DAC 25
#define CS_ADC 26

#define CS_METRONOME 32

#define MAX_AUDIO_CHANNELS 6
#define MAX_REC_CHANNELS 2
#define MAX_TOTAL_CHANNELS (MAX_AUDIO_CHANNELS+MAX_REC_CHANNELS)

#endif