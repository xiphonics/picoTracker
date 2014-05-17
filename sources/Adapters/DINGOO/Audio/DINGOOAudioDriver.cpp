#include "DINGOOAudioDriver.h"
#include "System/Console/Trace.h"
#include "Application/Model/Config.h"

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


DINGOOAudioDriver::DINGOOAudioDriver(AudioSettings &settings):SDLAudioDriver(settings)
{
  Config *config=Config::GetInstance();
  const char *volume=config->GetValue("VOLUME");
  volume_ = (volume) ? volume_=atoi(volume) : 65;
}

DINGOOAudioDriver::~DINGOOAudioDriver()
{
}

bool DINGOOAudioDriver::InitDriver()
{
  bool success = SDLAudioDriver::InitDriver();
  if (!success) return false;

  mixer_ = open("/dev/mixer", O_RDWR);
  if (mixer_==-1)
  {
     Trace::Error("Can t open volume mixer");
  }
   
   return true ;
}

void DINGOOAudioDriver::CloseDriver() 
{
  if (mixer_ != -1)
  {
    close(mixer_);
  }
  SDLAudioDriver::CloseDriver();
}

int DINGOOAudioDriver::GetVolume() 
{
	return volume_;
}

void DINGOOAudioDriver::SetVolume(int vol) 
{
	volume_=vol;
  Trace::Debug("Setting volume to %d",volume_);
  int realVol=(volume_<<8)+volume_ ;
  ioctl(mixer_, SOUND_MIXER_WRITE_VOLUME , &realVol);
}

