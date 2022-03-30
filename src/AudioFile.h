#ifndef AudioFile_h
#define AudioFile_h

#include "CircularBuffer.h"

#include "FS.h"
#include "SD.h"
#include <SPI.h>

#define DEBUG_FILE_MESSAGES true
#include "utils/Utilities.h"
// For eliptic arguments (...) in debug().
#include <stdarg.h>

#define PLAY_FREQUENCY 8000

struct AUDIO_FILE_INFO{
  char* fileName;
  uint32_t currentFileDirection;
  uint32_t size;
  uint8_t progress;
  String state;
  uint16_t frequency;
  uint8_t bitRes;
};

class AudioFile {
  public:
    static const uint8_t FILE_PAUSED  = 3;
    static const uint8_t FILE_PLAYING = 4;

    AudioFile();
    void open(char *filePath);
    void refreshBuffer();
    uint16_t getSample();
    void setTo(const uint8_t state);
    bool hasFileEnded();
    uint32_t getFileSize();
    AUDIO_FILE_INFO getAudioFileInfo();
    
  private:
    static const uint8_t WAV_FILE = 10;
    static const uint8_t RAW_FILE = 11;

    char* fileName;
    File dataFile;
    uint8_t fileType = RAW_FILE;
    uint32_t fileSize = 0;

    uint16_t audioFrequency;
    uint8_t audioResolution;
    uint8_t byteAudioResolution;
    uint8_t channelNumber;

    CircularBuffer buf;
    // The number of samples taken, aka. total number of iterations inside refreshBuffer().
    uint32_t sampleIndex = 0;
    uint32_t fileDirectionToBuffer = 0;
    uint16_t finalReadIndexOfFile = 0xFFFF;
    bool isOversampled = false;

    static const uint8_t FILE_OPENING = 0;
    static const uint8_t FILE_READY   = 1;
    static const uint8_t FILE_ENDED   = 2;
    static const uint8_t FILE_UNKNOWN_STATE = 0xFF;

    uint8_t fileState = FILE_UNKNOWN_STATE;

    void fetchAudioFileData();
    uint16_t read16();
    uint32_t read32();

    String getStatusString();

    void debug(const char* x, ... );
    void error(const char* x, ... );
};

#endif
