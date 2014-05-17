#include "SDLAudioDriver.h"
#include "System/Console/Trace.h"
#include "Services/Time/TimeService.h"
#include "System/System/System.h"
#include "Services/Midi/MidiService.h"

void sdl_callback(void *userdata, Uint8 *stream, int len) {
	SDLAudioDriver *sound=(SDLAudioDriver *)userdata ;
	sound->OnChunkDone(stream,len) ;
} ;

SDLAudioDriverThread::SDLAudioDriverThread(SDLAudioDriver *driver) {
	semaphore_=SysSemaphore::Create(0,4) ;
	driver_=driver ;
} ;

bool SDLAudioDriverThread::Execute() {
	while (!shouldTerminate()) {
		semaphore_->Wait() ;
		driver_->OnNewBufferNeeded();
	} ;
	SysSemaphore *semaphore=semaphore_ ;		
	semaphore_=0 ;
	delete semaphore ;
	return true ;
} ;

void SDLAudioDriverThread::Notify() {
	if (semaphore_) {
		semaphore_->Post() ;
	}
} ;

void SDLAudioDriverThread::RequestTermination() {
	SysThread::RequestTermination() ;
	// post to be sure we're not locked
	semaphore_->Post() ;
	// Wait for thread to finish
	SDL_Delay(10) ;
}

//-------------------------------------------------------------------------------------------------

SDLAudioDriver::SDLAudioDriver(AudioSettings &settings):AudioDriver(settings),
	unalignedMain_(0),
	miniBlank_(0)
{
	isPlaying_=false ;
	thread_=0 ;
}

SDLAudioDriver::~SDLAudioDriver() {
}

struct SDL_AudioSpec input ;
struct SDL_AudioSpec returned ;

bool SDLAudioDriver::InitDriver() {

  //set sound
  input.freq=44100 ;
  input.format=AUDIO_S16SYS ;
  input.channels=2 ;
  input.callback=sdl_callback ;
  input.samples=settings_.bufferSize_ ;
  input.userdata=this ;

  if (SDL_OpenAudio(&input,&returned) < 0 )
  {
  	Trace::Error("Couldn't open sdl audio: %s\n", SDL_GetError());
  	return false ;
  } 
  char bufferName[256] ;
  SDL_AudioDriverName(bufferName,256) ;

  fragSize_=returned.size ;
  // Allocates a rotating sound buffer
  unalignedMain_=(char *)SYS_MALLOC(fragSize_+SOUND_BUFFER_MAX) ;
  // Make sure the buffer is aligned
  mainBuffer_=(char *)((((int)unalignedMain_)+1)&(0xFFFFFFFC)) ;

  Trace::Log("AUDIO","%s successfully opened with %d samples",bufferName,fragSize_/4 ) ;

  // Create mini blank buffer in case of underruns

  miniBlank_=(char *)malloc(fragSize_) ;
  SYS_MEMSET(miniBlank_,0,fragSize_) ;

   return true ;
} ; 

void SDLAudioDriver::CloseDriver() {

	if (miniBlank_) {
		SYS_FREE(miniBlank_) ;
		miniBlank_=0 ;
	}

	if (unalignedMain_) {
		SYS_FREE (unalignedMain_) ;
		unalignedMain_=0 ;
	} ; 
	SDL_CloseAudio();
} ;

bool SDLAudioDriver::StartDriver() {

    thread_=new SDLAudioDriverThread(this) ;
    thread_->Start() ;

    short blank[4000] ;
    SYS_MEMSET(blank,0,4000) ;
    bufferPos_=0 ;
    bufferSize_=0 ;

	for (int i=0;i<settings_.preBufferCount_;i++) {
		AddBuffer((short *)miniBlank_,fragSize_/4) ;
	}
	if (settings_.preBufferCount_==0) {
		thread_->Notify() ;
	}

    SDL_PauseAudio(0);
	startTime_=SDL_GetTicks() ;
	
    return 1 ;
} ; 

void SDLAudioDriver::StopDriver() {
	if (thread_) {
		thread_->RequestTermination() ;
		SysThread *thread=thread_ ;
		thread_=0 ;
		SDL_PauseAudio(1);
		delete thread ;
 	} ;
} ;

double SDLAudioDriver::GetStreamTime() {
	return (SDL_GetTicks()-startTime_)/1000.0 ;
}

void SDLAudioDriver::OnChunkDone(Uint8 *stream,int len) {
  
      // Look if we have enough data in main buffer
      
       while (bufferSize_-bufferPos_<len) {

          // First move remaining bytes at the front
          memcpy(mainBuffer_,mainBuffer_+bufferPos_,bufferSize_-bufferPos_) ;

         // then get next queued buffer and copy data from it

    	 if (pool_[poolPlayPosition_].buffer_==0) {
    		 SYS_MEMCPY(mainBuffer_+bufferSize_-bufferPos_, miniBlank_,len);
    		 bufferSize_=bufferSize_-bufferPos_+len ;
		 
             bufferPos_=0 ;
         } else {

			memcpy(mainBuffer_+bufferSize_-bufferPos_, pool_[poolPlayPosition_].buffer_,pool_[poolPlayPosition_].size_);
    
             // Adapt buffer variables
    
    	     bufferSize_=bufferSize_-bufferPos_+pool_[poolPlayPosition_].size_ ;
             bufferPos_=0 ;
           
             SYS_FREE( pool_[poolPlayPosition_].buffer_) ;
    
             pool_[poolPlayPosition_].buffer_=0 ;
             poolPlayPosition_=(poolPlayPosition_+1)%SOUND_BUFFER_COUNT ;
	     	 if (thread_) thread_->Notify() ;

        }    	 
      }
      // Now dump audio to the device

      SYS_MEMCPY(stream,(short *)(mainBuffer_+bufferPos_), len); 
      onAudioBufferTick();
      bufferPos_+=len ;
}

int SDLAudioDriver::GetPlayedBufferPercentage() {
//	return 100-(bufferSize_-bufferPos_-fragSize_)*100/(bufferSize_-fragSize_) ;
	return 0 ;
} ;



