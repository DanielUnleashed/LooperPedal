#ifndef CircularBuffer_h
#define CircularBuffer_h

#include "Arduino.h"

#define BUFFER_REFRESH 2048
#define MAX_BUFFER_LENGTH 4096

class CircularBuffer{
  public:
    void put(uint16_t data);
    uint16_t get();
    uint16_t getFreeSpace();
    uint16_t getWriteIndex();
    uint16_t getReadIndex();
  
  private:
    uint16_t writeIndex = 0;
    uint16_t readIndex = 0;
    uint16_t buf[MAX_BUFFER_LENGTH];
};

#endif
