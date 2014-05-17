#ifndef _DUMMY_SOUND_H_
#define _DUMMY_SOUND_H_

#include "System/Sound/Sound.h"
class DummySound: public Sound {
public:
     // Sound implementation
	virtual bool Init() ; 
	virtual void Close();
	virtual bool Start() ; 
	virtual void Stop();
	virtual void QueueBuffer(char *buffer,int len) ;
	virtual int GetPlayedBufferPercentage() ;
    virtual int GetSampleRate() ;	
} ;
#endif
