#ifndef RECAudioFile_h
#define RECAudioFile_h

#include "audioFiles/AudioFile.h"
#include "audioFiles/WavFile.h"

#include "chip/adc.h"

#define REC_FOLDER "/rec/"

class RECAudioFile : public AudioFile {

    public:
        RECAudioFile();
        RECAudioFile(bool channel, ADC* inputADC);

        uint16_t getSample() override;
        void refreshBuffer() override; //Will actually save to SD.
        String getStatusString() override;
        bool hasFileEnded() override;

        void startRecording();
        void stopRecording();
        void undoRedoLastRecording();
        void writeToFile();

    private:
        ADC* adc;
        bool adcChannel;

        CircularBuffer recBuf[2];
        File recFiles[2];
        File tempMixingChannel;

        // Stores the current recording.
        File currentRecording;
        uint32_t recordingStartingDirection = 0;

        uint8_t recordingCount = 0;

        bool isRecording = false;
        bool laterPrevHasBeenMixed = true;
        bool playLastRecording = true;

        bool stopRecordingFlag = false;

        String generateFileName(uint8_t number);
        String generateFileName(uint8_t chA, uint8_t chB);
        void readFromSD(uint8_t channel);
        void readFromSD(uint8_t channel, uint32_t dir, uint16_t dataLength);
        void mixFromSD(uint8_t channel);
        void generateNewRECLayer();

};

#endif