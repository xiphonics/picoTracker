
#include "SDLAudio.h"
#include "SDLAudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"

SDLAudio::SDLAudio(AudioSettings &hints):Audio(hints) {
	hints_=hints;
}

SDLAudio::~SDLAudio() {
}

void SDLAudio::Init() {
	AudioSettings settings ;
	settings.audioAPI_=GetAudioAPI();

	settings.bufferSize_=GetAudioBufferSize() ;
	settings.preBufferCount_=GetAudioPreBufferCount() ;


   SDLAudioDriver *drv=new SDLAudioDriver(settings) ;
   AudioOut *out=new AudioOutDriver(*drv) ;
   Insert(out) ;
} ;

void SDLAudio::Close() {
     IteratorPtr<AudioOut>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         AudioOut &current=it->CurrentItem() ;
         current.Close() ;
     }
} ;

int SDLAudio::GetMixerVolume() {
	return 100 ;
} ;

void SDLAudio::SetMixerVolume(int volume) {
} ;
