
#include "DummyAudio.h"
#include "DummyAudioDriver.h"
#include "Services/Audio/AudioOut.h"
#include "System/io/Trace.h"

DummyAudio::DummyAudio() {
}

DummyAudio::~DummyAudio() {
}

void DummyAudio::Init() {
     DummyAudioDriver *drv=new DummyAudioDriver() ;
     AudioOut *out=new AudioOut(*drv) ;
     Insert(out) ;
} ;

void DummyAudio::Close() {
     IteratorPtr<AudioOut>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         AudioOut &current=it->CurrentItem() ;
         current.Close() ;
     }
} ;

