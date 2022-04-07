#include "CircularBuffer.h"

void CircularBuffer::put(uint16_t data){
  buf[writeIndex++] = data;
  if(writeIndex == MAX_BUFFER_LENGTH) writeIndex = 0;
}

uint16_t CircularBuffer::get(){
  uint16_t data = buf[readIndex++];
  if(readIndex == MAX_BUFFER_LENGTH) readIndex = 0;
  return data;
}

void CircularBuffer::get(uint16_t* outBuffer, uint16_t size){
  for(uint16_t i = 0; i < size; i++){
    outBuffer[i] = get();
  }
}

uint16_t CircularBuffer::getFreeSpace(){
  uint16_t dist = 0;
  if(writeIndex < readIndex) dist = readIndex - writeIndex;
  else dist = MAX_BUFFER_LENGTH - writeIndex + readIndex;
  return dist;
}

uint16_t CircularBuffer::getWrittenSpace(){
  uint16_t dist = 0;
  if(writeIndex < readIndex) dist = MAX_BUFFER_LENGTH - readIndex + writeIndex;
  else dist = writeIndex - readIndex;
  return dist;
}

uint16_t CircularBuffer::getWriteIndex(){
  return writeIndex;
}

uint16_t CircularBuffer::getReadIndex(){
  return readIndex;
}
