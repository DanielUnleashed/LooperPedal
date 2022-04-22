#ifndef AudioFile_h
#define AudioFile_h

#include "Arduino.h"
#include "CircularBuffer.h"

#include "utils/Utilities.h"

#include "FS.h"
#include "SD.h"
#include <SPI.h>

#define SD_FILE_ID 0
#define REC_FILE_ID 1
//#define MIDI_FILE 2

struct AUDIO_FILE_INFO{
  const char* fileName;
  uint32_t currentFileDirection;
  uint32_t size;
  uint8_t progress;
  String state;
  uint8_t bitRes;
};

class AudioFile{
  public: 
      static const uint8_t FILE_PAUSED  = 3;
      static const uint8_t FILE_PLAYING = 4;

      uint8_t ID;

      virtual uint16_t getSample() = 0;
      virtual void refreshBuffer() = 0;
      virtual String getStatusString() = 0;
      virtual bool hasFileEnded() = 0;

      void setStatus(uint8_t stat);
      uint8_t getStatus();
      uint32_t getCurrentFileDirection();
      uint32_t getFileSize();
      AUDIO_FILE_INFO getAudioFileInfo();
      bool is(uint8_t askID);

      String getFileName();
      String getFileLocation();

  private:

  protected:
      String fileLoc;
      uint8_t fileStatus = 0xFF;
      
      uint32_t fileDirectionToBuffer = 0;
      uint32_t fileSize = 0;
      CircularBuffer buf;

      uint8_t audioResolution = 16;

};

#endif