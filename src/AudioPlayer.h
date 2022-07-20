#ifndef AudioPlayer_h
#define AudioPlayer_h

#include "audioFiles/SDAudioFile.h"
#include "audioFiles/RECAudioFile.h"
#include "defines.h"

#define DEBUG_AUDIOPLAYER_MESSAGES true
#include "utils/Utilities.h"

#include "chip/DAC.h"
#include "chip/ADC.h"

#include "Metronome.h"

#include "UI/Input/DebounceButton.h"

class AudioPlayer{
  public:
    static void play();
    static void pause();

    static void addSDAudioFile(char* filePath);
    static void addRECAudioFile(bool channel);
    static void memoryTask(void* funcParams);
    static void statusMonitorTask(void* funcParams);
    static void audioProcessingTask(void* funcParams);
    static void begin();

    static void SDBoot();

    static bool isPlaying;
    static bool isRecording;
    
  private:
    static AudioFile* audioChannels[MAX_TOTAL_CHANNELS];
    static uint8_t channelsUsed;    // Total sum of channels.

    static uint8_t SDChannelPos[MAX_AUDIO_CHANNELS];
    static uint8_t RECChannelPos[MAX_REC_CHANNELS];
    static uint8_t audioChannelsUsed;
    static uint8_t recChannelsUsed;

    static uint8_t longestChannel;  // Index of the longest audio track in audioChannels.

    static CircularBuffer globalBuf;

    static portMUX_TYPE timerMux;
    static hw_timer_t *timer;

    static DAC dac;
    static ADC adc;

    static Metronome metronome;

    static TaskHandle_t audioProcessingTaskHandle;
    static TaskHandle_t statusMonitorTaskHandle;
    static TaskHandle_t memoryTaskHandle;

    static void setAllTo(const uint8_t audioFileID, const uint8_t state);

    static void IRAM_ATTR frequencyTimer();

    static RECAudioFile* getRECAudioFile(uint8_t index);
    static SDAudioFile* getSDAudioFile(uint8_t index);

    static void debug(const char* x, ...);
    static void error(const char* x, ...);
};

#endif