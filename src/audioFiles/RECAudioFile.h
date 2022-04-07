#ifndef RECAudioFile_h
#define RECAudioFile_h

#include "audioFiles/AudioFile.h"

class RECAudioFile : public AudioFile {

    public:
        uint16_t getSample() override;
        void refreshBuffer() override;
        String getStatusString() override;
        bool hasFileEnded() override;

    private:

};

#endif