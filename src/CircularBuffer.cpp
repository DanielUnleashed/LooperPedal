#include "CircularBuffer.h"

bool CircularBuffer::put(uint16_t data){
  if(getFreeSpace() == 0) return false;
  buf[writeIndex++] = data;
  if(writeIndex == AUD_MAX_BUFFER_LENGTH) writeIndex = 0;
  return true;
}

uint16_t CircularBuffer::get(){
  uint16_t data = buf[readIndex++];
  if(readIndex == AUD_MAX_BUFFER_LENGTH) readIndex = 0;
  return data;
}

uint16_t CircularBuffer::getFreeSpace(){
  uint16_t dist = 0;
  if(writeIndex >= readIndex) dist = AUD_MAX_BUFFER_LENGTH - writeIndex + readIndex;
  else dist = readIndex - writeIndex;
  return dist;
}

uint16_t CircularBuffer::getWriteIndex(){
  return writeIndex;
}

uint16_t CircularBuffer::getReadIndex(){
  return readIndex;
}
