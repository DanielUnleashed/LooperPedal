#ifndef AudioPlayer_h
#define AudioPlayer_h

#include "AudioFile.h"

#define DEBUG_AUDIOPLAYER_MESSAGES true
#include "utils/Utilities.h"
// For eliptic arguments (...) in debug().
#include <stdarg.h>

//Temporary to simulate DAC output
#include <driver/dac.h>
#include <esp_task_wdt.h>

#define MAX_AUDIO_CHANNELS 5
#define PLAY_BIT_RESOLUTION 8
#define PLAY_FREQUENCY 8000

#define PLAY_BYTE_RESOLUTION \
        ((PLAY_BIT_RESOLUTION>>3) + ((PLAY_BIT_RESOLUTION&0x0007) > 0) ? 1 : 0)
#define PLAY_BYTE_RATE PLAY_FREQUENCY*PLAY_BYTE_RESOLUTIONPLA

class AudioPlayer{
  public:
    static uint32_t PLAY_TIME_START;

    static void begin();
    static void addAudioFile(AudioFile file);
    static void addAudioFile(char* filePath);
    static void memoryTask(void* funcParams);
    static void statusMonitorTask(void* funcParams);
    static void audioProcessingTask(void* funcParams);
    static void start();
    
  private:
    static const uint8_t PLAY_ONCE     = 2;
    static const uint8_t PLAY_LOOP     = 3;
  
    static AudioFile audioChannels[MAX_AUDIO_CHANNELS];
    static uint8_t channelsUsed;    // Number of channels used.
    static uint8_t longestChannel;  // Index of the longest audio track in audioChannels.

    static CircularBuffer globalBuf;

    static SemaphoreHandle_t semPlayFreq;
    static portMUX_TYPE timerMux;
    static hw_timer_t *timer;
    static uint8_t playMode;

    static void setAllTo(const uint8_t state);

    static void IRAM_ATTR frequencyTimer();
    static void debug(const char* x, ...);
    static void error(const char* x, ...);
};

#endif
