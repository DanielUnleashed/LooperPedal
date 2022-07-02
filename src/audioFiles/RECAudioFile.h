#ifndef RECAudioFile_h
#define RECAudioFile_h

#include "audioFiles/AudioFile.h"
#include "audioFiles/WavFile.h"

#include "chip/adc.h"

#define REC_FOLDER "/rec/"

class RECAudioFile : public AudioFile {

    public:
        String recordingName;

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

        /* Stores the buffer data of the last and past mixed recordings:
            - 0: Last recording.
            - 1: Past mixed recordings.
        */
        CircularBuffer recBuf[2];

        /* Files of the last and past mixed recordings:
            - 0: Last recording.
            - 1: Past mixed recordings.
            - 2: Last mixed recording, will close when file 1 is mixed.
        */
        File recFiles[3];
        File tempMixingChannel;

        // This is the recording that is being played at the moment.
        File currentRecording;

        uint8_t recordingCount = 0;

        bool isRecording = false;
        bool laterPrevHasBeenMixed = true;
        bool playLastRecording = true;

        bool stopRecordingFlag = false;

        String generateFileName(uint8_t number);
        String generateFileName(uint8_t chA, uint8_t chB);
        void readFromSD(uint8_t channel);
        void readFromSD(uint8_t channel, uint32_t dir, uint16_t dataLength);

        /*Mixes two audio files. Let current iteration be i, then this function will mix (i-2) with
        (i-3). Iteration i-3 stores the mixing of recordings from i = 0 to i-3.*/
        void mixFromSD();
        void generateNewRECLayer();

};

#endif