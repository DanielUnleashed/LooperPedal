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

void CircularBuffer::put(File* file, uint16_t size){
  if(writeIndex + size/2 >= MAX_BUFFER_LENGTH){ // If the circular buffer overflows, then...
    // Save the beggining bytes at the end of the buffer.
    uint16_t spaceLeft = (MAX_BUFFER_LENGTH - writeIndex)<<1;
    file->read((uint8_t*)(buf+writeIndex), (size_t) spaceLeft);
    
    // Go to the position where the read array left off.
    file->seek(file->position() + spaceLeft);

    // Continue reading and writing to the beggining of the circular buffer.
    uint16_t remainingBytes = size - spaceLeft;
    file->read((uint8_t*)(buf), (size_t) remainingBytes);
    writeIndex = remainingBytes/2;
  }else{
    file->read((uint8_t*)(buf+writeIndex), (size_t) size);
    writeIndex += size/2;
  }
}

uint16_t CircularBuffer::get(){
  uint16_t data = buf[readIndex++];
  if(readIndex == MAX_BUFFER_LENGTH) readIndex = 0;
  return data;
}

void CircularBuffer::get(uint16_t* outBuffer, uint16_t size){
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

void CircularBuffer::copyToFile(File* file, uint16_t size){
  if(readIndex + size/2 >= MAX_BUFFER_LENGTH){
    uint16_t spaceLeft = (MAX_BUFFER_LENGTH - readIndex) << 1;
    file->write((uint8_t*) (buf+readIndex), spaceLeft);

    file->seek(file->position() + spaceLeft);

    uint16_t remainingBytes = size - spaceLeft;
    file->write((uint8_t*)buf, remainingBytes);
    readIndex = remainingBytes/2;
  }else{
    file->write((uint8_t*)(buf+readIndex), size);
    readIndex += size/2;
  }
}

uint16_t CircularBuffer::getFreeSpace(){
  if(writeIndex < readIndex) return readIndex - writeIndex;
  else return MAX_BUFFER_LENGTH - writeIndex + readIndex;
}

uint16_t CircularBuffer::getWrittenSpace(){
  if(writeIndex < readIndex) return MAX_BUFFER_LENGTH - readIndex + writeIndex;
  else return writeIndex - readIndex;
}

uint16_t CircularBuffer::getWriteIndex(){
  return writeIndex;
}

uint16_t CircularBuffer::getReadIndex(){
  return readIndex;
}
