#include "DummyAudioOut.h"
#include "Application/Player/SyncMaster.h" // Should be installable
#include "Services/Time/TimeService.h"
#include "Services/Audio/AudioDriver.h"

static bool threadon=false ;

DummyOutThread::DummyOutThread(DummyAudioOut *out) {
	out_=out ;
} ;

bool DummyOutThread::Execute() {

	while(!this->shouldTerminate()) {
		out_->SendPulse() ;
		// let it breathe a bit
		TimeService::GetInstance()->Sleep(1) ;
	}
	return false ;

} ;

DummyAudioOut::DummyAudioOut() {
    SetOwnership(false) ;
	thread_=0 ;
}

DummyAudioOut::~DummyAudioOut() {
}

void DummyAudioOut::Trigger() {
	int sampleCount=getPlaySampleCount() ;  
    AudioMixer::Render(primarySoundBuffer_,sampleCount) ;
} ;

bool DummyAudioOut::Init() {
	primarySoundBuffer_=(int *)SYS_MALLOC(MIX_BUFFER_SIZE*sizeof(fixed)/2) ;
	return (primarySoundBuffer_!=0) ;
} ;
void DummyAudioOut::Close() {
	SAFE_FREE(primarySoundBuffer_) ;
}

bool DummyAudioOut::Start() {
	thread_=new DummyOutThread(this) ;
	thread_->Start() ;
	return true ;
} ;

void DummyAudioOut::Stop() {
	thread_->RequestTermination() ;
	while (!thread_->IsFinished()) ;
	SAFE_DELETE(thread_) ;
} ;

void DummyAudioOut::SendPulse()
{
  SetChanged() ;
  AudioDriver::Event event(AudioDriver::Event::ADET_BUFFERNEEDED);
  NotifyObservers(&event) ;
} ;

std::string DummyAudioOut::GetAudioAPI() {
	return std::string("Internal") ;
} ;

std::string DummyAudioOut::GetAudioDevice() {
	return std::string("Dummy") ;
} ;

int DummyAudioOut::GetAudioBufferSize() {
	return 0 ;
} ;

int DummyAudioOut::GetAudioRequestedBufferSize() {
	return 0 ;
}

int DummyAudioOut::GetAudioPreBufferCount() {
	return 0 ;
} ;

double DummyAudioOut::GetStreamTime() {
	return 0 ;
};
