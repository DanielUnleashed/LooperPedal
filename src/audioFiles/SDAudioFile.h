#ifndef SDAudioFile_h
#define SDAudioFile_h

#include "CircularBuffer.h"

#include "FS.h"
#include "SD.h"
#include <SPI.h>

#include "AudioFile.h"
#include "WavFile.h"
#include "defines.h"

#define DEBUG_FILE_MESSAGES true
#include "utils/Utilities.h"
// For eliptic arguments (...) in debug().
#include <stdarg.h>

#define PUSH_BUTTON_1 34
#define PUSH_BUTTON_2 35
#define PUSH_BUTTON_3 36
#define PUSH_BUTTON_4 39

class SDAudioFile : public AudioFile {
  public:
    static const uint8_t FILE_PAUSED  = 3;
    static const uint8_t FILE_PLAYING = 4;

    static const String PROCESSED_FOLDER;

    SDAudioFile();
    bool open(char *filePath);
    
    void calculateTotalIteration(uint32_t maxFileSize);

    uint16_t getSample() override;
    void refreshBuffer() override;
    String getStatusString() override;
    bool hasFileEnded() override;

  private:
    uint8_t byteAudioResolution;

    File dataFile;

    uint32_t fileDirectionToBuffer = 0;
    uint16_t finalReadIndexOfFile = 0xFFFF;

    uint16_t currentIteration = 0;
    uint16_t maxIterations = 1;

    static const uint8_t FILE_OPENING = 0;
    static const uint8_t FILE_READY   = 1;
    static const uint8_t FILE_ENDED   = 2;
    static const uint8_t FILE_UNKNOWN_STATE = 0xFF;

    bool fetchSDAudioFileData();

    static void IRAM_ATTR ISR_BUTTON_1();
    static void IRAM_ATTR ISR_BUTTON_2();
    static void IRAM_ATTR ISR_BUTTON_3();
    static void IRAM_ATTR ISR_BUTTON_4();

    void debug(const char* x, ... );
    void error(const char* x, ... );
};

#endif
