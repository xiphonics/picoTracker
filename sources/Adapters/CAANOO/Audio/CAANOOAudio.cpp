
#include "CAANOOAudio.h"
#include "CAANOOAudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"
#include "System/Console/Trace.h"

CAANOOAudio::CAANOOAudio(AudioSettings &settings):Audio(settings) {
}

CAANOOAudio::~CAANOOAudio() {
}

void CAANOOAudio::Init() {
	AudioSettings settings ;
        settings.audioAPI_=GetAudioAPI();

        settings.bufferSize_=GetAudioBufferSize() ;
        settings.preBufferCount_=GetAudioPreBufferCount() ;

     CAANOOAudioDriver *drv=new CAANOOAudioDriver(settings) ;
     AudioOutDriver *out=new AudioOutDriver(*drv) ;
     Insert(out) ;
} ;

void CAANOOAudio::Close() {
     IteratorPtr<AudioOut>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         AudioOut &current=it->CurrentItem() ;
         current.Close() ;
     }
} ;

void CAANOOAudio::SetMixerVolume(int v) {
     AudioOutDriver *out=(AudioOutDriver*)GetFirst();
     if (out) {
          CAANOOAudioDriver *drv=(CAANOOAudioDriver *)out->GetDriver() ;
          drv->SetVolume(v);
     }    
}     

int CAANOOAudio::GetMixerVolume() {
     AudioOutDriver *out=(AudioOutDriver*)GetFirst();
     if (out) {
          CAANOOAudioDriver *drv=(CAANOOAudioDriver *)out->GetDriver() ;
          return drv->GetVolume();
     }    
     return 0;
}    
