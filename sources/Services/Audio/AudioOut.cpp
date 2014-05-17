#include "AudioOut.h"
#include "Application/Player/SyncMaster.h"

AudioOut::AudioOut():AudioMixer("AudioOut"),sampleOffset_(0) {
} ;

AudioOut::~AudioOut() {
} ;


int AudioOut::getPlaySampleCount() {
	sampleOffset_+=SyncMaster::GetInstance()->GetPlaySampleCount() ;
	int count=int(sampleOffset_) ;
	sampleOffset_-=count ;
	return count ;
} ;