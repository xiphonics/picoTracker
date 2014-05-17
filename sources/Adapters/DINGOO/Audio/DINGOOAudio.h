#pragma once

#include "Services/Audio/Audio.h"
#include "DINGOOAudioDriver.h"

class DINGOOAudio: public Audio
{
public:
	DINGOOAudio(AudioSettings &hints) ; // Allow for different default size for different platform
	~DINGOOAudio() ;
	virtual void Init() ;
	virtual void Close() ;
	virtual int GetMixerVolume() ;
	virtual void SetMixerVolume(int volume) ;
private:
	DINGOOAudioDriver *drv;	
	AudioSettings hints_ ;
};
