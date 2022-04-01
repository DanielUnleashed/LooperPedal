#ifndef AudioPlayer_h
#define AudioPlayer_h

#include "AudioFile.h"
#include "defines.h"

#define DEBUG_AUDIOPLAYER_MESSAGES true
#include "utils/Utilities.h"
// For eliptic arguments (...) in debug().
#include <stdarg.h>

#include "chip/DAC.h"
#define CS_DAC 15

#define MAX_AUDIO_CHANNELS 5

class AudioPlayer{
  public:
    static uint32_t PLAY_TIME_START;

    static void play();
    static void pause();

    static void addAudioFile(AudioFile file);
    static void addAudioFile(char* filePath);
    static void memoryTask(void* funcParams);
    static void statusMonitorTask(void* funcParams);
    static void audioProcessingTask(void* funcParams);
    static void begin();
    
  private:
    static const uint8_t PLAY_ONCE     = 2;
    static const uint8_t PLAY_LOOP     = 3;

    static AudioFile audioChannels[MAX_AUDIO_CHANNELS];
    static uint8_t channelsUsed;    // Number of channels used.
    static uint8_t longestChannel;  // Index of the longest audio track in audioChannels.

    static CircularBuffer globalBuf;

    static portMUX_TYPE timerMux;
    static hw_timer_t *timer;
    static uint8_t playMode;
    static bool isPlaying;

    static DAC dac;

    static TaskHandle_t audioProcessingTaskHandle;
    static TaskHandle_t statusMonitorTaskHandle;
    static TaskHandle_t memoryTaskHandle;

    static void SDBoot();
    static void setAllTo(const uint8_t state);

    static void IRAM_ATTR frequencyTimer();

    static void IRAM_ATTR ISR_BUTTON_1();
    static void IRAM_ATTR ISR_BUTTON_2();
    static void IRAM_ATTR ISR_BUTTON_3();
    static void IRAM_ATTR ISR_BUTTON_4();

    static void debug(const char* x, ...);
    static void error(const char* x, ...);
};

#endif
