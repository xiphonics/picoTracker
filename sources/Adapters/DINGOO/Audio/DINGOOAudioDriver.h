#pragma once

#include "Services/Audio/AudioDriver.h"
#include "Adapters/SDL/Audio/SDLAudioDriver.h"

class DINGOOAudioDriver:public SDLAudioDriver 
{
public:
  DINGOOAudioDriver(AudioSettings &settings) ;
  virtual ~DINGOOAudioDriver() ;

	virtual bool InitDriver() ; 
	virtual void CloseDriver();

	void SetVolume(int vol);
	int GetVolume();

private:
  int volume_;
  long mixer_;
};

