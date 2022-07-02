#ifndef CircularBuffer_h
#define CircularBuffer_h

#include "Arduino.h"
#include "FS.h"

#define BUFFER_REFRESH 1024
#define MAX_BUFFER_LENGTH 4096

class CircularBuffer{
  public:
    // Puts single item inside the circular buffer.
    void put(uint16_t data);
    // Puts an array inside the circular buffer (optimized).
    void put(uint8_t* data, uint16_t size);
    // Directly puts the information from the file to the circular array (highly optimized!).
    void put(File* file, uint16_t size);

    uint16_t get();
    void get(uint16_t* outbuffer, uint16_t size);
    uint16_t getFreeSpace();
    uint16_t getWrittenSpace();
    uint16_t getWriteIndex();
    uint16_t getReadIndex();
  
  private:
    uint16_t writeIndex = 0;
    uint16_t readIndex = 0;
    uint16_t buf[MAX_BUFFER_LENGTH];
};

#endif
