#ifndef _CAANOOAUDIO_H_
#define _CAANOOAUDIO_H_

#include "Services/Audio/Audio.h"

class CAANOOAudio: public Audio {
public:
    CAANOOAudio(AudioSettings &settings) ;
    ~CAANOOAudio() ;
    virtual void Init() ;
    virtual void Close() ;
	virtual int GetMixerVolume()  ;
	virtual void SetMixerVolume(int volume) ; 
    };
#endif
