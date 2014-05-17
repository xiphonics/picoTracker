#ifndef _RTAUDIO_STUB_H_
#define _RTAUDIO_STUB_H_

#include "Services/Audio/Audio.h"
#include "Externals/RTAudio/RtAudio.h"

class RTAudioStub: public Audio {
public:
	RTAudioStub(AudioSettings &settings) ;
    ~RTAudioStub() ;
    virtual void Init() ;
    virtual void Close() ;
	virtual int GetSampleRate()  ;
private:
	RtAudio::Api api_ ;
	std::string selectedDevice_ ;
	int index_ ; // index of the device to open
	int sampleRate_ ;
};
#endif
