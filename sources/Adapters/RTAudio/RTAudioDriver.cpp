
#include "RTAudioDriver.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "Services/Time/TimeService.h"
#include "Services/Audio/Audio.h"
#include "Application/Player/SyncMaster.h"
#include "Adapters/W32/Midi/W32MidiService.h"
#include <string.h>

int callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
	RTAudioDriver *sound=(RTAudioDriver *)userData ;
	sound->fillBuffer((short *)outputBuffer,nBufferFrames) ;
	return 0 ;
}

RTAudioDriverThread::RTAudioDriverThread(RTAudioDriver *driver) {
	semaphore_=SysSemaphore::Create(0,10000) ;
	driver_=driver ;
	percentage_= 0 ;
} ;

bool RTAudioDriverThread::Execute() {

	int bufferSize=Audio::GetInstance()->GetAudioBufferSize() ;
	float cycleTime=bufferSize/44100.0f ;
	TimeService *ts=TimeService::GetInstance();
	while (!shouldTerminate()) {
		semaphore_->Wait() ;
		float before=float(ts->GetTime()) ;
		driver_->OnNewBufferNeeded() ;
		float delta=float(ts->GetTime()-before) ; // in secs
		percentage_=int(delta/cycleTime*100);
	} ;
	delete semaphore_ ;
	return true ;
} ;

int RTAudioDriverThread::GetPlayedBufferPercentage() {
	return percentage_ ;
}

void RTAudioDriverThread::Notify() {
	if (semaphore_) {
		semaphore_->Post() ;
	}
} ;


RTAudioDriver::RTAudioDriver(RtAudio::Api api,AudioSettings &settings):AudioDriver(settings),
	unalignedMain_(0),
	miniBlank_(0),
	audio_(api)
{
	isPlaying_=false ;
	thread_=0 ;
} ;

bool RTAudioDriver::InitDriver() {

	RtAudio::StreamParameters params ;
	int deviceID=-1 ;
	std::string	deviceName=settings_.audioDevice_;
	for (uint i=0;i<audio_.getDeviceCount();i++)
  {
		RtAudio::DeviceInfo info = audio_.getDeviceInfo(i);
		if (info.name==deviceName)
    {
			deviceID=i ;
			break ;
		}
	}

	if (deviceID<0)
  {
		deviceID=audio_.getDefaultOutputDevice() ;
	}

	params.deviceId=deviceID ;
	params.nChannels=2 ;
	params.firstChannel=0 ;

	unsigned int sampleRate = 44100;
	unsigned int bufferFrames = settings_.bufferSize_ ; // in samples

	try 
  {
		audio_.openStream( &params, NULL, RTAUDIO_SINT16,
		                sampleRate, &bufferFrames, &callback, (void *)this);
	}
  catch (RtError &e)
  {
		Trace::Error("Error opening audio output stream: %s",e.getMessage().c_str()) ;
		return false ;
	};

	fragSize_=bufferFrames*4 ;
	Trace::Log("AUDIO","RTAudio device %s successfully open - buffer=%d",deviceName.c_str(),bufferFrames) ;

	settings_.audioDevice_=deviceName ;

	// Allocates a rotating sound buffer
	unalignedMain_=(char *)SYS_MALLOC(fragSize_+SOUND_BUFFER_MAX) ;
	// Make sure the buffer is aligned
	mainBuffer_=(char *)((((int)unalignedMain_)+1)&(0xFFFFFFFC)) ;

	// Create mini blank buffer in case of underruns

	miniBlank_=(char *)malloc(fragSize_) ;
	SYS_MEMSET(miniBlank_,0,fragSize_) ;

	return true ;
} ;

void RTAudioDriver::CloseDriver() {

	audio_.closeStream();

	if (miniBlank_)
  {
		SYS_FREE(miniBlank_);
		miniBlank_=0 ;
	}

	if (unalignedMain_) 
  {
		SYS_FREE (unalignedMain_);
		unalignedMain_=0;
	} ; 
} ;

bool RTAudioDriver::StartDriver() 
{
	thread_=new RTAudioDriverThread(this);
  thread_->Start() ;

  SYS_MEMSET(miniBlank_,0,fragSize_);
  bufferPos_=0 ;
  bufferSize_=0 ;

	for (int i=0;i<settings_.preBufferCount_;i++)
  {
		AddBuffer((short *)miniBlank_,fragSize_/4);
		MidiService::GetInstance()->AdvancePlayQueue();
	}

	if (settings_.preBufferCount_==0)
  {
		thread_->Notify();
	}
	try
  {
		audio_.startStream();
	}
  catch (RtError &e) 
  {
		Trace::Error("Error starting output stream: %s",e.getMessage().c_str()) ;
		return false ;
	};
	return true ;
}

void RTAudioDriver::StopDriver()
{
	audio_.abortStream();
} ;

double RTAudioDriver::GetStreamTime()
{
	return audio_.getStreamTime();
}

void RTAudioDriver::fillBuffer(short *stream,int frameCount)
{
  int len = frameCount*4;

  // Look if we have enough data in main buffer
    
  while (bufferSize_-bufferPos_ < len)
  {
    // First move remaining bytes at the front
    // then get next queued buffer and copy data from it

    if (pool_[poolPlayPosition_].buffer_ == 0)
    { 
    // underrun, let's fill the buffer with blank and bail out
     SYS_MEMSET(stream, 0, len);
     return ;
    }
    else
    {
      memcpy(mainBuffer_,mainBuffer_+bufferPos_,bufferSize_-bufferPos_) ;
		  memcpy(mainBuffer_+bufferSize_-bufferPos_, pool_[poolPlayPosition_].buffer_,pool_[poolPlayPosition_].size_);
    
			MidiService::GetInstance()->Flush() ;

			 // Adapt buffer variables
    
      bufferSize_ = bufferSize_-bufferPos_ + pool_[poolPlayPosition_].size_ ;
      bufferPos_ = 0 ;

      SYS_FREE( pool_[poolPlayPosition_].buffer_) ;

      pool_[poolPlayPosition_].buffer_ = 0 ;
      poolPlayPosition_ = (poolPlayPosition_+1)%SOUND_BUFFER_COUNT ;
      thread_->Notify() ;
     }    	 
  }
  SYS_MEMCPY(stream,(short *)(mainBuffer_+bufferPos_), len); 
  onAudioBufferTick();
  bufferPos_+=len ;	  
}

int RTAudioDriver::GetPlayedBufferPercentage()
{
	return thread_->GetPlayedBufferPercentage() ;
}
