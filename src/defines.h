#ifndef defines_h
#define defines_h

//#define LAUNCH_CONSOLE

//#define USE_BOTH_ADC_CHANNELS

// Branch inside AudioPlayer.cpp
#define PASS_AUDIO_INPUT_DURING_RECORDING

#define PLAY_FREQUENCY 16000
#define BIT_RES 16

#define SCREEN_FPS 50
#define DTFT_WIDTH 160
#define DTFT_HEIGHT 128

// If more were to be added, add to the init() method inside DebounceButton.cpp
// another call to attachInterrupt with the correspondant number.
#define TOTAL_BUTTONS 4
static const uint8_t PUSH_BUTTON[TOTAL_BUTTONS] = {34,35,36,39};

// Same as before. If more were to be added, go to RotaryEncoder.cpp
// It only differs that two interrupts have to be added, one for each
// channel of the rotary encoder. 
// If the rotatory encoder doesn't have button, fill with number 0xFF the
// correspondent number inside ROTARY_BUTTONS array.
#define TOTAL_ROTARY_ENCODERS 1
#define TOTAL_ROTARY_BUTTONS 1
static const uint8_t ROTARY_ENCODERS[TOTAL_ROTARY_ENCODERS<<1] = {};
static const uint8_t ROTARY_BUTTONS[TOTAL_ROTARY_ENCODERS] = {};

#define CS_DAC 25
#define CS_ADC 26

#define CS_METRONOME 32

#define MAX_AUDIO_CHANNELS 6
#define MAX_REC_CHANNELS 2
#define MAX_TOTAL_CHANNELS (MAX_AUDIO_CHANNELS+MAX_REC_CHANNELS)

#endif