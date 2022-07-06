#ifndef CircularBuffer_h
#define CircularBuffer_h

#include "Arduino.h"
#include "SD.h"

// The buffer will refresh when the distance between read and written uint16_t (2 bytes) are greater than thee.
#define BUFFER_REFRESH 1024
// Length of the circular buffer in uint16_t.
#define MAX_BUFFER_LENGTH 4096

class CircularBuffer{
  public:
    // Puts single item inside the circular buffer.
    void put(uint16_t data);
    // Puts an array inside the circular buffer (optimized).
    void put(uint8_t* data, uint16_t size);
    // Directly puts the information from the file to the circular array (highly optimized!).
    void put(File* file, uint16_t size);

    // Returns the next item inside the circular buffer.
    uint16_t get();
    // Copies the elements from the circular buffer to a given outbuffer.
    void get(uint16_t* outbuffer, uint16_t size);

    // Writes directly the content of the buffer to a SD file.
    void copyToFile(File* file, uint16_t size);
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
