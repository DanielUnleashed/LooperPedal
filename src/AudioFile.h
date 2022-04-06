#ifndef AudioFile_h
#define AudioFile_h

#include "CircularBuffer.h"

#include "FS.h"
#include "SD.h"
#include <SPI.h>

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

struct AUDIO_FILE_INFO{
  const char* fileName;
  uint32_t currentFileDirection;
  uint32_t size;
  uint8_t progress;
  String state;
  uint8_t bitRes;
};

class AudioFile {
  public:
    static const uint8_t FILE_PAUSED  = 3;
    static const uint8_t FILE_PLAYING = 4;

    static const String PROCESSED_FOLDER;

    String fileName;

    AudioFile();
    bool open(char *filePath);
    void refreshBuffer();
    uint16_t getSample();
    void setTo(const uint8_t state);
    bool hasFileEnded();
    uint32_t getFileSize();
    uint32_t getCurrentFileDirection();
    AUDIO_FILE_INFO getAudioFileInfo();
    
  private:
    File dataFile;
    uint32_t fileSize = 0;

    uint8_t audioResolution;
    uint8_t byteAudioResolution;

    CircularBuffer buf;
    uint32_t fileDirectionToBuffer = 0;
    uint16_t finalReadIndexOfFile = 0xFFFF;

    static const uint8_t FILE_OPENING = 0;
    static const uint8_t FILE_READY   = 1;
    static const uint8_t FILE_ENDED   = 2;
    static const uint8_t FILE_UNKNOWN_STATE = 0xFF;

    uint8_t fileState = FILE_UNKNOWN_STATE;

    void fetchAudioFileData();

    String getStatusString();

    static void IRAM_ATTR ISR_BUTTON_1();
    static void IRAM_ATTR ISR_BUTTON_2();
    static void IRAM_ATTR ISR_BUTTON_3();
    static void IRAM_ATTR ISR_BUTTON_4();

    void debug(const char* x, ... );
    void error(const char* x, ... );
};

#endif
