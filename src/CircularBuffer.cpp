#include "CircularBuffer.h"

void CircularBuffer::put(uint16_t data){
  buf[writeIndex++] = data;
  if(writeIndex == MAX_BUFFER_LENGTH) writeIndex = 0;
}

void CircularBuffer::put(uint8_t* data, uint16_t size){
  //for(uint16_t i = 0; i < size; i+=2)
  //  put(data[i] | (data[i+1] << 8));
  if(writeIndex + size/2 >= MAX_BUFFER_LENGTH){
    uint16_t spaceLeft = (MAX_BUFFER_LENGTH - writeIndex)<<1;
    memmove(buf+writeIndex, data, (size_t)spaceLeft);
    uint16_t remainingBytes = size - spaceLeft;
    memmove(buf, data+spaceLeft, (size_t)remainingBytes);
    writeIndex = remainingBytes/2;
  }else{
    memmove(buf+writeIndex, data, (size_t)size);
    writeIndex += size/2;
  }
}

uint16_t CircularBuffer::get(){
  uint16_t data = buf[readIndex++];
  if(readIndex == MAX_BUFFER_LENGTH) readIndex = 0;
  return data;
}

void CircularBuffer::get(uint16_t* outBuffer, uint16_t size){
//  for(uint16_t i = 0; i < size; i++)
//    outBuffer[i] = get();
  if(readIndex + size/2 >= MAX_BUFFER_LENGTH){
    uint16_t spaceLeft = (MAX_BUFFER_LENGTH - readIndex) << 1;
    memmove(outBuffer, buf+readIndex, spaceLeft);
    uint16_t remainingBytes = size - spaceLeft;
    memmove(outBuffer+(spaceLeft/2), buf, remainingBytes);
    readIndex = remainingBytes/2;
  }else{
    memmove(outBuffer, buf+readIndex, size);
    readIndex += size/2;
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
