#ifndef CircularBuffer_h
#define CircularBuffer_h

#include "Arduino.h"

#define BUFFER_REFRESH 1024

class CircularBuffer{
  public:
    static const uint16_t AUD_MAX_BUFFER_LENGTH = 8192;
  
    void put(uint16_t data);
    uint16_t get();
    uint16_t getFreeSpace();
    uint16_t getWriteIndex();
    uint16_t getReadIndex();
  
  private:
    uint16_t writeIndex = 0;
    uint16_t readIndex = 0;
    uint16_t buf[AUD_MAX_BUFFER_LENGTH];
};

#endif
