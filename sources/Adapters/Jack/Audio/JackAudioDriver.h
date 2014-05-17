#ifndef _JACK_OUT_DRIVER_H_
#define _JACK_OUT_DRIVER_H_

#include "Services/Audio/AudioDriver.h"
#include <jack/jack.h>
#include "System/Process/Process.h"
#include "Adapters/Jack/Client/JackClient.h"

class JackAudioDriver ;

class JackAudioDriverThread: public SysThread {
public:
	JackAudioDriverThread(JackAudioDriver *driver) ;
	virtual ~JackAudioDriverThread() {} ;
	virtual bool Execute() ;
	void Notify() ;
private:
	JackAudioDriver *driver_ ;
	SysSemaphore *semaphore_ ;
} ;

class JackAudioDriver:public AudioDriver,public JackProcessor {
public:
    JackAudioDriver(jack_client_t *client,AudioSettings &settings) ;
    virtual ~JackAudioDriver() ;

     // Sound implementation
	virtual bool InitDriver() ; 
	virtual void CloseDriver();
	virtual bool StartDriver() ; 
	virtual void StopDriver();
	virtual int GetPlayedBufferPercentage() { return 0 ; }
	virtual int GetSampleRate() { return 44100 ; } ;
	virtual bool Interlaced() { return true ; } ;	
	virtual double GetStreamTime() ;
	jack_port_t *GetPort() ;
	virtual void ProcessCallback(jack_nframes_t nframes) ;
	void dumpBuffer(jack_default_audio_sample_t *dstL,jack_default_audio_sample_t *dstR,short *src,int sampleCount) ;
private:
	bool running_ ;
	short *tempBuffer_ ;
	short *current_ ;
	unsigned int available_ ;
	jack_client_t *client_ ;
	jack_port_t *portL_;
	jack_port_t *portR_;
    JackAudioDriverThread *thread_ ;
	jack_time_t startTime_ ;
} ;
#endif
