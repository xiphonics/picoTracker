#ifndef _SDL_AUDIO_H_
#define _SDL_AUDIO_H_

#include "Services/Audio/Audio.h"

class SDLAudio: public Audio {
public:
	SDLAudio(AudioSettings &hints) ; // Allow for different default size for different platform
	~SDLAudio() ;
	virtual void Init() ;
	virtual void Close() ;
	virtual int GetMixerVolume() ;
	virtual void SetMixerVolume(int volume) ;
private:
	AudioSettings hints_ ;
};
#endif
