#ifndef WavFile_h
#define WavFile_h

#include "FS.h"
#include "SD.h"
#include <SPI.h>

#include "defines.h"
#include "utils/Utilities.h"

#include "AudioFile.h"

#define WAV_TRANSFORM_CHANNEL_NUM   1
#define WAV_TRANSFORM_BITS          16
#define DATA_COPY_BUFFER_SIZE       512

struct WAV_FILE_INFO {
    String fileName;
    uint32_t dataSize;
};

// Makes this struct be the size it is especified by each uint.
#pragma pack(push, 1)
struct  WAV_HEADER {
    // RIFF Chunk
    uint8_t     RIFF_ID[4];         // "RIFF"
    uint32_t    RIFF_DataSize;      // RIFF Chunk data Size
    uint8_t     RIFF_TYPE_ID[4];    // "WAVE"
    // format sub-chunk
    uint8_t     fmt_ID[4];          // "fmt "
    uint32_t    fmt_DataSize;       // Size of the format chunk
    uint16_t    fmt_FormatTag;      // Format_Tag 1=PCM
    uint16_t    channelNum;         // 1=Mono 2=Stereo
    uint32_t    sampleRate;         // Sampling Frequency in Hz
    uint32_t    byteRate;           // Byte rate = Chnls. * samples/sec * bits per sample/8
    uint16_t    blockAlign;         // Chnls. * bits per sample/8
    uint16_t    bitsPerSample;    
    /* "data" sub-chunk */
    uint8_t     data_ID[4];         // "data"
    uint32_t    data_DataSize;      // Size of the audio data
};
#pragma pack(pop)

class WavFile {
    public:
        String wavLoc;

        WavFile(String fileLoc);
        WAV_FILE_INFO processToRawFile();
        static void processToWavFile(AudioFile* rawFile);
    private:
        WAV_HEADER wavInfo;
        uint32_t wavFileDataSize;

        void getHeader();
        void printHeader(File outFile);
        bool processAudioData(File outFile);
        bool directCopy(File outFile);

        File wavFile; 
        uint16_t read16();
        uint32_t read32();
        uint8_t readByte();
};

#endif