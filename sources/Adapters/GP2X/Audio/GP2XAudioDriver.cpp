
#include "GP2XAudioDriver.h"
#include "System/System/System.h"
#include "Services/Midi/MidiService.h"
#include "Application/Model/Config.h"
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <semaphore.h>


static unsigned long gp2x_dev[2] ;
static pthread_t gp2x_sound_thread,gp2x_engine_thread;

#define DEV_DSP 0
#define DEV_MIXER 1

static volatile unsigned long  gp2x_sound_pausei,gp2x_exit ;

void gp2x_sound_pause(int yes) { gp2x_sound_pausei=yes; }

static sem_t sem; 

// sound thread

static void *gp2x_sound_play(void *blah)
{
  gp2x_sound_pausei=1 ;
  GP2XAudioDriver *sound=(GP2XAudioDriver *)blah ;

// try changing the local thread priority
 
  struct sched_param sp;
  pthread_t self=pthread_self() ;

  int scheduling ;
  
  pthread_getschedparam(self,&scheduling,&sp) ;
  int priority=sched_get_priority_max(scheduling) ;
  sp.sched_priority=priority;
  pthread_setschedparam(self,scheduling,&sp) ;
   

 while(!gp2x_exit)  {
      if(!gp2x_sound_pausei) {
		  sound->OnChunkDone() ;
       }
  }
  return NULL;

}

// engine thread

static void *gp2x_engine_loop(void *blah) {
 
 // try changing the local thread priority
 
  struct sched_param sp;
  pthread_t self=pthread_self() ;

  int scheduling ;
  
  pthread_getschedparam(self,&scheduling,&sp) ;
  int priority=sched_get_priority_max(scheduling) ;
  sp.sched_priority=priority;
//  pthread_setschedparam(self,scheduling,&sp) ;
  
  sem_init( &sem, PTHREAD_PROCESS_PRIVATE, 0 ); 
  
  GP2XAudioDriver *sound=(GP2XAudioDriver *)blah ;

  while (!gp2x_exit) {
    sem_wait( &sem ); // - (block if 0) 
    sem_destroy( &sem );
    sound->NewBuffer() ;
   }     
 
  return NULL ;
}


GP2XAudioDriver::GP2XAudioDriver(AudioSettings &settings):AudioDriver(settings) {

  Config *config=Config::GetInstance() ;
    
  const char *dspDevice=config->GetValue("GP2X_DSP") ;
  const char *mixDevice=config->GetValue("GP2X_MIXER") ;

  if (dspDevice==0) dspDevice="/dev/dsp" ;
  if (mixDevice==0) mixDevice="/dev/mixer" ;
  
  gp2x_dev[DEV_DSP] = open(dspDevice,   O_WRONLY );
  gp2x_dev[DEV_MIXER] = open(mixDevice, O_RDWR);
  gp2x_exit=0 ;

  //set sound

  int rate=44100 ;
  int bits=16 ;
  int stereo=1 ;

  ioctl(gp2x_dev[DEV_DSP], SNDCTL_DSP_SPEED,  &rate);
  ioctl(gp2x_dev[DEV_DSP], SNDCTL_DSP_SETFMT, &bits);
  ioctl(gp2x_dev[DEV_DSP], SNDCTL_DSP_STEREO, &stereo);
  //

  audio_buf_info abi;
  int arg= 0x80000|7 ;
  ioctl(gp2x_dev[DEV_DSP], SNDCTL_DSP_SETFRAGMENT, &arg) ;
  ioctl(gp2x_dev[DEV_DSP], SNDCTL_DSP_GETOSPACE, &abi);
  fragSize_=abi.fragsize ;

  printf("Using frag size=%d",fragSize_) ;
	
  pthread_create( &gp2x_engine_thread, NULL, gp2x_engine_loop, this);
 
  pthread_create( &gp2x_sound_thread, NULL, gp2x_sound_play, this);

}

GP2XAudioDriver::~GP2XAudioDriver() {
 
   gp2x_exit=1 ;

   close(gp2x_dev[DEV_DSP]) ;
   close(gp2x_dev[DEV_MIXER]) ;
}

bool GP2XAudioDriver::InitDriver() {

   unalignedMain_=(char *)SYS_MALLOC(fragSize_+SOUND_BUFFER_MAX) ;
   mainBuffer_=(char *)((((int)unalignedMain_)+1)&(0xFFFFFFFC)) ;

   volume_=65;
   Config *config=Config::GetInstance() ;
   const char *volume=config->GetValue("VOLUME") ;

   if (volume) {
      volume_=atoi(volume) ;
   }  

   int realVol=(volume_<<8)+volume_ ;
   ioctl(gp2x_dev[DEV_MIXER], MIXER_WRITE(SOUND_MIXER_PCM) , &realVol);

  // Create mini blank buffer for underrun
  
   miniBlank_=(char *)malloc(fragSize_) ;
   memset(miniBlank_,0,fragSize_) ;
	 
   return true ;
} 

void GP2XAudioDriver::SetVolume(int v) {
	volume_=v ;
	int realVol=(volume_<<8)+volume_ ;
    ioctl(gp2x_dev[DEV_MIXER], MIXER_WRITE(SOUND_MIXER_PCM) , &realVol);

}

int GP2XAudioDriver::GetVolume() {
	return volume_ ;
}

void GP2XAudioDriver::CloseDriver() {
	SAFE_FREE(unalignedMain_) ;
	SAFE_FREE(miniBlank_) ;
}

bool GP2XAudioDriver::StartDriver() {
 
  isPlaying_=true ;
  
  short blank[1000] ;
	SYS_MEMSET(blank,0,2000) ;
	bufferPos_=0 ;
	bufferSize_=0 ;

  ticksBeforeMidi_=4 ;
  
  for (int i=0;i<ticksBeforeMidi_;i++) {
    AddBuffer(blank,500) ;
	}

  streamSampleTime_=0 ;
  gp2x_sound_pause(0);
  sem_post(&sem) ;

	return 1 ;
} 

void GP2XAudioDriver::StopDriver() {
     gp2x_sound_pause(1);
     isPlaying_=false ;
}


void GP2XAudioDriver::OnChunkDone() {
    

	// Look if we have enoug data in main buffer
      
       if (bufferSize_-bufferPos_<fragSize_) {

		// Are we in underrun ?
	   
    	 if (pool_[poolPlayPosition_].buffer_==0) {
			// quickly send miniblank and baill out
			int written=write(gp2x_dev[DEV_DSP],(short *)miniBlank_, fragSize_); 
			return ;
			
		} else { 

             sem_post(&sem) ;

			// First move remaining bytes at the front
			SYS_MEMCPY(mainBuffer_,mainBuffer_+bufferPos_,bufferSize_-bufferPos_) ;
			 
             if (ticksBeforeMidi_) {
               ticksBeforeMidi_-- ;
             } else {
               MidiService::GetInstance()->Flush() ;
             }
    		 SYS_MEMCPY(mainBuffer_+bufferSize_-bufferPos_, pool_[poolPlayPosition_].buffer_,pool_[poolPlayPosition_].size_);
    
             // Adapt buffer variables
    
    		 bufferSize_=bufferSize_-bufferPos_+pool_[poolPlayPosition_].size_ ;
             bufferPos_=0 ;
           
             SYS_FREE( pool_[poolPlayPosition_].buffer_) ;
    
             pool_[poolPlayPosition_].buffer_=0 ;
             poolPlayPosition_=(poolPlayPosition_+1)%SOUND_BUFFER_COUNT ;
        }    	 
      }

      // Now dump audio to the device

      int written=write(gp2x_dev[DEV_DSP],(short *)(mainBuffer_+bufferPos_), fragSize_); 
      bufferPos_+=fragSize_ ;
	  streamSampleTime_+=fragSize_ ;
}

int GP2XAudioDriver::GetPlayedBufferPercentage() {
	return 100-(bufferSize_-bufferPos_-fragSize_)*100/(bufferSize_-fragSize_) ;
}

void GP2XAudioDriver::NewBuffer() {
     AudioDriver::OnNewBufferNeeded() ;
}

double GP2XAudioDriver::GetStreamTime() {
	return (streamSampleTime_)/44100.0f ;
}