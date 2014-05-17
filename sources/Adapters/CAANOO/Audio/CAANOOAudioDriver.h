#ifndef _CAANOO_SOUND_H_
#define _CAANOO_SOUND_H_

#include "Services/Audio/AudioDriver.h"


class CAANOOAudioDriver:public AudioDriver {
public:
    CAANOOAudioDriver(AudioSettings &settings) ;
    virtual ~CAANOOAudioDriver() ;
    
     // Sound implementation
	virtual bool InitDriver() ; 
	virtual void CloseDriver();
	virtual bool StartDriver() ; 
	virtual void StopDriver();
	virtual int GetPlayedBufferPercentage() ;
	virtual int GetSampleRate() { return 44100 ; } ;
	virtual bool Interlaced() { return true ; } ;
	// Additional
	void OnChunkDone() ;
	void SetVolume(int v) ;
	int GetVolume() ;
    void NewBuffer() ;
	virtual double GetStreamTime() ;	
private:
	AudioSettings settings_ ;
    int fragSize_ ;
	char *unalignedMain_ ;
    char *mainBuffer_ ;
    char *miniBlank_ ;
    int bufferPos_ ;
    int bufferSize_ ;
	int volume_ ;
	int ticksBeforeMidi_ ;
	unsigned long streamSampleTime_ ;	
} ;
#endif
