#ifndef _NO_JACK_
#include "JackAudioDriver.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
//#include "Adapters/Jack/Midi/JackMidiService.h"

JackAudioDriverThread::JackAudioDriverThread(JackAudioDriver *driver) {
	semaphore_=0 ;
	driver_=driver ;
} ;

bool JackAudioDriverThread::Execute() {
	semaphore_=SysSemaphore::Create(0,4) ;
	while (!shouldTerminate()) {
		semaphore_->Wait() ;
		driver_->SetChanged() ;
		driver_->NotifyObservers() ;
	} ;
	delete semaphore_ ;
	return true ;
} ;

void JackAudioDriverThread::Notify() {
	if (semaphore_) {
		semaphore_->Post() ;
	}
} ;

JackAudioDriver::JackAudioDriver(jack_client_t *client,AudioSettings &settings):AudioDriver(settings) {
// Jack driver currently ignores the settings
	client_=client ;
	portL_ = jack_port_register (client, "mixL",JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	portR_ = jack_port_register (client, "mixR",JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	thread_=0 ;
} ;

JackAudioDriver::~JackAudioDriver() {
} ;

bool JackAudioDriver::InitDriver() {

	Trace::Debug("[J]: Init driver") ;

// Get output port and connect straight to the default system output

    thread_=new JackAudioDriverThread(this) ;
    thread_->Start() ;

	const char **ports;

	Trace::Debug("[J]: Get ports") ;
	if ((ports = jack_get_ports (client_, NULL, NULL, 
                               JackPortIsPhysical|JackPortIsInput)) == NULL) {
		Trace::Debug("Cannot find any physical playback ports\n");
		return false ;
	}
	
	Trace::Debug("[J]: connect L") ;
	if (jack_connect (client_, jack_port_name (portL_), ports[0])) {
	    fprintf (stderr, "cannot connect left out\n");
	}
	Trace::Debug("[J]: connect R") ;
	if (jack_connect (client_, jack_port_name (portR_), ports[1])) {
	    fprintf (stderr, "cannot connect right out\n");
	}
	running_=false ;
	Trace::Debug("[J]: free ports") ;
	free (ports);

	return true ;
} ;

void JackAudioDriver::CloseDriver() {

	if (thread_) {
		thread_->RequestTermination() ;
		thread_->Notify() ;
		while(!thread_->IsFinished()) {
		} ;
	} ;
	thread_=0 ;
} ;

bool JackAudioDriver::StartDriver() {
	Trace::Debug("[J]: start driver") ;
	tempBuffer_=0 ;
	available_=0 ;
	startTime_ = jack_get_time() ;
	return true ;
} ;

void JackAudioDriver::StopDriver() {
	SAFE_FREE(tempBuffer_) ;
} ;

void JackAudioDriver::dumpBuffer(jack_default_audio_sample_t *dstL,jack_default_audio_sample_t *dstR,short *src,int frameCount) {

	for (int i=0;i<frameCount;i++) {
		*dstL++=(float(*src++)/32767.0) ;
		*dstR++=(float(*src++)/32767.0) ;
	}
} ;

double JackAudioDriver::GetStreamTime() {
	return (jack_get_time()-startTime_)/1000.0/1000.0 ;
}

void JackAudioDriver::ProcessCallback(jack_nframes_t frames) {

//	Trace::Dump("[J]: process frames %d",frames) ;
	jack_nframes_t nframes=frames ;

 // Get the out buffer

	jack_default_audio_sample_t *outL = 
                (jack_default_audio_sample_t *) 
                jack_port_get_buffer (portL_, nframes);

	jack_default_audio_sample_t *outR = 
                (jack_default_audio_sample_t *) 
                jack_port_get_buffer (portR_, nframes);

//	JackMidiService *jms=(JackMidiService *)MidiService::GetInstance() ;
//	jms->OnNewFrame() ;

	if (!tempBuffer_) {
		tempBuffer_=(short *)malloc(SOUND_BUFFER_MAX) ;
		current_=tempBuffer_ ;
		available_=0 ;
	}

	if (!hasData()) {
		SYS_MEMSET(outL,0,nframes*sizeof(jack_default_audio_sample_t)) ;
		SYS_MEMSET(outR,0,nframes*sizeof(jack_default_audio_sample_t)) ;
		if (thread_) thread_->Notify() ;
	} else {
		while (nframes>0) {

			int read=0 ;
	
			if (!available_) {
				thread_->Notify() ;
	//			jms->SetFrameOffset(frames-nframes) ;
	//			jms->Flush() ;
				if (pool_[poolPlayPosition_].buffer_!=0) {
					available_=(pool_[poolPlayPosition_].size_)/2/sizeof(short)  ;
					SYS_MEMCPY(tempBuffer_,pool_[poolPlayPosition_].buffer_,pool_[poolPlayPosition_].size_);
					SYS_FREE( pool_[poolPlayPosition_].buffer_) ;
					pool_[poolPlayPosition_].buffer_=0 ;
					poolPlayPosition_=(poolPlayPosition_+1)%SOUND_BUFFER_COUNT ;
					current_=tempBuffer_ ;
				} else {
					SYS_MEMSET(tempBuffer_,0,nframes*sizeof(short *)*2) ;
					available_=nframes ;
					current_=tempBuffer_ ;
					Trace::Debug("[J]: Underrun detected") ;
				}
			}
			read=(available_>nframes)?nframes:available_ ;
			dumpBuffer(outL,outR,current_,read) ;
			available_-=read ;
			outL+=read ;
			outR+=read ;
			nframes-=read ;
			current_+=read*2 ;
		}
    }
} ;
#endif
