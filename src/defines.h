#ifndef defines_h
#define defines_h

//#define LAUNCH_CONSOLE

//#define USE_BOTH_ADC_CHANNELS

//#define PASS_AUDIO_INPUT_DURING_RECORDING

#define PLAY_FREQUENCY 22050

// Total input buttons. If more were to be added, add it to the static array in DebounceButton.cpp
// Also add to the init() another call to attachInterrupt with num.
#define TOTAL_BUTTONS 4
#define PUSH_BUTTON_1 34
#define PUSH_BUTTON_2 35
#define PUSH_BUTTON_3 36
#define PUSH_BUTTON_4 39

#define CS_DAC 25
#define CS_ADC 26

#define CS_METRONOME 32

#define MAX_AUDIO_CHANNELS 6
#define MAX_REC_CHANNELS 2
#define MAX_TOTAL_CHANNELS (MAX_AUDIO_CHANNELS+MAX_REC_CHANNELS)

#endif