#ifndef _JACK_AUDIO_H_
#define _JACK_AUDIO_H_

#include "Services/Audio/Audio.h"
#include "jack/jack.h"

class JackAudio: public Audio {
public:
	JackAudio(AudioSettings &hints) ;
	~JackAudio() ;
	virtual void Init() ;
	virtual void Close() ;
};
#endif
