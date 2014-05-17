#ifndef _DUMMY_AUDIODRIVER_H_
#define _DUMMY_AUDIODRIVER_H_

#include "Services/Audio/AudioDriver.h"

class DummyAudioDriver: public AudioDriver {
public:
     // Sound implementation
	virtual bool InitDriver() ; 
	virtual void CloseDriver();
	virtual bool StartDriver() ; 
	virtual void StopDriver();
	virtual int GetPlayedBufferPercentage() ;
	virtual int GetSampleRate() ;
	virtual bool Interlaced() { return true ; } ;
} ;
#endif
