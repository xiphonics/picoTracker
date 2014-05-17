#ifndef _GP2X_SOUND_H_
#define _GP2X_SOUND_H_

#include "Services/Audio/AudioDriver.h"


class GP2XAudioDriver:public AudioDriver {
public:
    GP2XAudioDriver(AudioSettings &settings) ;
    virtual ~GP2XAudioDriver() ;
    
     // Sound implementation
	virtual bool InitDriver() ; 
	virtual void CloseDriver();
	virtual bool StartDriver() ; 
	virtual void StopDriver();
	virtual int GetPlayedBufferPercentage() ;
	virtual int GetSampleRate() { return 44100 ; } ;
	virtual bool Interlaced() { return true ; } ;
	virtual double GetStreamTime() ;
	// Additional
	void OnChunkDone() ;
	void SetVolume(int v) ;
	int GetVolume() ;
    void NewBuffer() ;
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
