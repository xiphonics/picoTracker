#ifndef _NO_JACK_
#include "JackAudio.h"
#include "JackAudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"
#include "Adapters/Jack/Client/JackClient.h"

int jack_srate (jack_nframes_t nframes, void *arg) {
	Trace::Debug("Jack sample rate is now %lu/sec\n", nframes);
	return 0;
}

JackAudio::JackAudio(AudioSettings &hints):Audio(hints) {

}

JackAudio::~JackAudio() {
}



void JackAudio::Init() {

	JackClient *jc=JackClient::GetInstance() ;
	jack_client_t *client=jc->GetClient() ; NAssert(client) ;
	if (client==0) return  ;

	jack_set_sample_rate_callback (client, jack_srate, 0);

//	Create the driver

     JackAudioDriver *drv=new JackAudioDriver(client,settings_) ;
     AudioOut *out=new AudioOutDriver(*drv) ;
     Insert(out) ;
	jc->Insert(drv) ;

// Return, the rest will be done later

	if (jack_activate (client)) {
		Trace::Debug("Failed to activate client ?") ;
	}


} ;

void JackAudio::Close() {
     IteratorPtr<AudioOut>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         AudioOut &current=it->CurrentItem() ;
         current.Close() ;
     }
} ;
#endif
