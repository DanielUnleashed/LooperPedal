#ifndef defines_h
#define defines_h

//#define LAUNCH_CONSOLE

// Still not fully implemented...
//#define USE_BOTH_ADC_CHANNELS

// Branch inside AudioPlayer.cpp
#define PASS_AUDIO_INPUT_DURING_RECORDING

//#define LAUNCH_SPLASHSCREEN_AT_BOOT_UP

// Will add LED animations (makes the pedal way to slow, maybe the problem happens only when connected to PC because low power?).
#define METRONOME_LEDS

// If left defined all animatoins will play, functionality of everything else will remain
// untouched. If left undefined, DisplayOverlay animations will not play and all "good-looking"
// animations will be disabled.
#define ENABLE_DISPLAY_ANIMATIONS

#define PLAY_FREQUENCY 22050
#define BIT_RES 16

#define SCREEN_FPS 25.0

#define TILES_X 6
#define TILES_Y 4
#define TASKBAR_HEIGHT 20

// If more were to be added, add to the init() method inside DebounceButton.cpp
// another call to attachInterrupt with the correspondant number.
#define TOTAL_BUTTONS 6
static const uint8_t PUSH_BUTTON[TOTAL_BUTTONS] = {34,35,36,39, 32,4};

#define ANALOG_INPUT_PIN 4
// With current version, max numer of analog buttons is 9, because base ten is used in the string.
// If more were to be added, simply change the base of the code that is added to the iterative function
// to identify the button code.
#define TOTAL_ANALOG_BUTTONS 4
// The maximum number of buttons that can be pressed at any time.
#define MAX_ANALOG_COMBINATIONS 3
static const uint32_t INPUT_RESISTANCE =  1e3;
static const uint32_t ANALOG_RESISTANCES[TOTAL_ANALOG_BUTTONS] = {10,100,1000,10000};


// Same as before. If more rotary encoders were to be added, go to RotaryEncoder.cpp
// It only differs that two interrupts have to be added, one for each
// channel of the rotary encoder. 
// If the rotatory encoder doesn't have button, fill with number 0xFF the
// correspondent number inside ROTARY_BUTTONS array. This buttons are located inside DebounceButton,
// and are treated as such.
#define TOTAL_ROTARY_ENCODERS 1
#define TOTAL_ROTARY_BUTTONS 1
static const uint8_t ROTARY_ENCODERS[TOTAL_ROTARY_ENCODERS<<1] = {16,17};
static const uint8_t ROTARY_BUTTONS[TOTAL_ROTARY_ENCODERS] = {22};

// IO Pins
#define CS_DAC 25
#define CS_ADC 26

#define CS_METRONOME 33

//Audio Player defines.
#define MAX_AUDIO_CHANNELS 6
#define MAX_REC_CHANNELS 2
#define MAX_TOTAL_CHANNELS (MAX_AUDIO_CHANNELS+MAX_REC_CHANNELS)

#endif