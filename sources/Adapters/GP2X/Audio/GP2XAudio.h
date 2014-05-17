#ifndef _GP2XAUDIO_H_
#define _GP2XAUDIO_H_

#include "Services/Audio/Audio.h"

class GP2XAudio: public Audio {
public:
    GP2XAudio(AudioSettings &settings) ;
    ~GP2XAudio() ;
    virtual void Init() ;
    virtual void Close() ;
	virtual int GetMixerVolume()  ;
	virtual void SetMixerVolume(int volume) ; 
    };
#endif
