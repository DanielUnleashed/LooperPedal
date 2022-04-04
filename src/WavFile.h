#ifndef WavFile_h
#define WavFile_h

#include "FS.h"
#include "SD.h"
#include <SPI.h>

#include "defines.h"
#include "utils/Utilities.h"

#define WAV_TRANSFORM_CHANNEL_NUM   1
#define WAV_TRANSFORM_BITS          16
#define DATA_COPY_BUFFER_SIZE       512

struct WAV_FILE_INFO {
    String fileName;
    uint32_t dataSize;
};

struct  WAV_HEADER {
    // RIFF Chunk
    uint8_t*        RIFF_ID;        // RIFF
    uint32_t        RIFF_DataSize;    // RIFF Chunk data Size
    uint8_t*        RIFF_TYPE_ID;    // WAVE
    // format sub-chunk
    uint8_t*        fmt_ID;       // fmt
    uint32_t        fmt_DataSize;   // Size of the format chunk
    uint16_t        fmt_FormatTag;         //  format_Tag 1=PCM
    uint16_t        channelNum;       //  1=Mono 2=Stereo
    uint32_t        sampleRate;        // Sampling Frequency in (44100)Hz
    uint32_t        byteRate;          // Byte rate
    uint16_t        blockAlign;        // 4
    uint16_t        bitsPerSample;    // 16
    /* "data" sub-chunk */
    uint8_t*        data_ID;       // data
    uint32_t        data_DataSize;   // Size of the audio data
};

class WavFile {
    public:
        WavFile(String loc);
        String fileLoc;
        WAV_FILE_INFO processToRawFile();
    private:
        WAV_HEADER wavInfo;

        void getHeader();
        void printHeader(File outFile);
        bool processAudioData(File outFile);
        bool directCopy(File outFile);

        File wavFile; 
        uint16_t read16();
        uint32_t read32();
        uint8_t* readBytes(uint8_t length);
};

#endif