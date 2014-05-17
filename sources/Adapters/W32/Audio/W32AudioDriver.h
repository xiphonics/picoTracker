#ifndef _W32_SOUND_H_
#define _W32_SOUND_H_

#include "Services/Audio/AudioDriver.h"
#include "Foundation/T_Stack.h"
#include <windows.h>

struct W32SoundBuffer {
	WAVEHDR *wavHeader_ ;
	int index_ ; // poolindex
} ;

class W32AudioDriver: public AudioDriver {
public:
	W32AudioDriver(int index,AudioSettings &settings) ;
     // Audio implementation
	virtual bool InitDriver() ; 
	virtual void CloseDriver();
	virtual bool StartDriver() ; 
	virtual void StopDriver();
	virtual int GetPlayedBufferPercentage() { return 0 ;} ;
    virtual int GetSampleRate() { return 44100; } ;	
    virtual bool Interlaced() { return true ; } ;
    virtual double GetStreamTime() ;
	// Additional
	void OnChunkDone(W32SoundBuffer *) ;
protected:
	void sendNextChunk(bool notify=true) ;
  void clearPlayedChunk(W32SoundBuffer *) ;
private:
	double streamTime_ ;
  int index_ ;
	int ticksBeforeMidi_ ;
	HWAVEOUT waveOut_ ;
} ;
#endif
