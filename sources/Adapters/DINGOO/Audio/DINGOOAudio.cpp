
#include "DINGOOAudio.h"
#include "DINGOOAudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"

DINGOOAudio::DINGOOAudio(AudioSettings &hints):Audio(hints) 
{
	hints_=hints;
}

DINGOOAudio::~DINGOOAudio() 
{
}

void DINGOOAudio::Init() 
{
  drv=new DINGOOAudioDriver(hints_) ;
  AudioOut *out=new AudioOutDriver(*drv) ;
  Insert(out) ;
}

void DINGOOAudio::Close() 
{
  IteratorPtr<AudioOut>it(GetIterator()) ;
  for (it->Begin();!it->IsDone();it->Next())
  {
     AudioOut &current=it->CurrentItem() ;
     current.Close() ;
  }
}

int DINGOOAudio::GetMixerVolume() 
{
	return drv->GetVolume();
}

void DINGOOAudio::SetMixerVolume(int volume) 
{
  drv->SetVolume(volume);
}
