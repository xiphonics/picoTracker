#ifndef _W32AUDIO_H_
#define _W32AUDIO_H_

#include "Services/Audio/Audio.h"

class W32Audio: public Audio {
public:
    W32Audio(AudioSettings &settings) ; // use NULL for midi mapper
    ~W32Audio() ;
    virtual void Init() ;
    virtual void Close() ;
	virtual int GetSampleRate()  ;
private:
	int index_ ; // index of the device to open
	int sampleRate_ ;
};
#endif
