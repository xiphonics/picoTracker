#ifndef _RTAUDIO_DRIVER_H_
#define _RTAUDIO_DRIVER_H_

#include "Services/Audio/AudioDriver.h"
#include "Externals/RTAudio/RtAudio.h"
#include "System/Process/Process.h"
#include "Foundation/Observable.h"
#include "System/Timer/Timer.h"
#include "Services/Midi/MidiService.h"

class RTAudioDriver ;

class RTAudioDriverThread: public SysThread {
public:
	RTAudioDriverThread(RTAudioDriver *driver) ;
	virtual ~RTAudioDriverThread() {} ;
	virtual bool Execute() ;
	void Notify() ;
	virtual int GetPlayedBufferPercentage()  ;
private:
	RTAudioDriver *driver_ ;
	SysSemaphore *semaphore_ ;
	int percentage_ ;
} ;


class RTAudioDriver: public AudioDriver {
public:
	RTAudioDriver(RtAudio::Api api,AudioSettings &settings) ;
     // Audio implementation
	virtual bool InitDriver() ; 
	virtual void CloseDriver();
	virtual bool StartDriver(); 
	virtual void StopDriver();
	virtual int GetPlayedBufferPercentage()  ;
	virtual double GetStreamTime() ;
  virtual int GetSampleRate() { return 44100; } ;	
  virtual bool Interlaced() { return true ; } ;
	void fillBuffer(short *buffer,int frameCount) ;

private:
	RtAudio audio_ ;
  int fragSize_ ;
  char *unalignedMain_ ;
  char *mainBuffer_ ;
  char *miniBlank_ ;
  int bufferPos_ ;
  int bufferSize_ ;
  RTAudioDriverThread *thread_ ;
} ;
#endif
