
#include "W32AudioDriver.h"
#include <math.h>
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "Services/Midi/MidiService.h"
#include <assert.h>

void CALLBACK winmm_cback(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {

	switch(uMsg) {
		case WOM_OPEN:
			Trace::Debug("Windows MM Audio device opened");
			break;
		case WOM_CLOSE:
			Trace::Debug("Windows MM Audio device closed");
			break;
		case WOM_DONE:
		//	try {
				W32AudioDriver *sound=(W32AudioDriver *)dwInstance ;
				WAVEHDR *hdr=(WAVEHDR *)dwParam1 ;
				W32SoundBuffer *sb=(W32SoundBuffer *)hdr->dwUser ;
				sound->OnChunkDone(sb) ;
		//	} catch(...) {
		//		Trace::Debug("Error treating sound block") ;
		//	}
			break;
	};
}

//
W32AudioDriver::W32AudioDriver(int index,AudioSettings &settings):AudioDriver(settings) {
	waveOut_=0 ;
	index_=index ;
}

bool W32AudioDriver::InitDriver() {
    
	WAVEFORMATEX fx;
	MMRESULT res;


	fx.wFormatTag = WAVE_FORMAT_PCM;
	fx.wBitsPerSample = 16;
	fx.nChannels = 2;
	fx.nSamplesPerSec = 44100;
    fx.nBlockAlign = (WORD)((fx.nChannels * fx.wBitsPerSample) / 8); 
    fx.nAvgBytesPerSec = (fx.nSamplesPerSec * fx.nBlockAlign); 
    fx.cbSize = 0; 
	if((res = waveOutOpen(&waveOut_, index_, &fx, (DWORD)winmm_cback,(DWORD)this,(DWORD) CALLBACK_FUNCTION )) != MMSYSERR_NOERROR) {
        Trace::Error("W32AudioDriver::Init:Failed") ;
        waveOut_=0 ;
		return false ;
	}

	// allocate buffers & wav headers

	for (int i=0;i<SOUND_BUFFER_COUNT;i++) {
		W32SoundBuffer *sb=new W32SoundBuffer() ;
		sb->wavHeader_ =(WAVEHDR *) malloc(sizeof(WAVEHDR));
		pool_[i].driverData_=sb ;
	}
	return true ;
} ; 

void W32AudioDriver::CloseDriver() {

	waveOutClose(waveOut_);
	for (int i=0;i<SOUND_BUFFER_COUNT;i++) {
		W32SoundBuffer *sb=(W32SoundBuffer *)pool_[i].driverData_ ;
		SAFE_FREE(sb) ;
		pool_[i].driverData_=0 ;
    }
} ;

bool W32AudioDriver::StartDriver() {

	 MMRESULT res;

  if (waveOut_==0) return false ;
	
	res=waveOutReset(waveOut_) ;
	if (res!=MMSYSERR_NOERROR) {
		Trace::Error("Could not reset wave out") ;
		return false ;
	};
	
    // Queue a few buffers to get the driver running
        
    short blank[8000] ;
	SYS_MEMSET(blank,0,8000*sizeof(short)) ;

    int prebuffer=8 ;
    
    ticksBeforeMidi_=0;
    for (int i=0;i<prebuffer;i++) {
		AddBuffer(blank,4000) ;
        sendNextChunk(i==prebuffer-1) ;
 	}

	streamTime_=0 ;
  return true ;
} ; 

void W32AudioDriver::StopDriver() {
	if (waveOut_) {
	   waveOutReset(waveOut_) ;
    }

} ;

double W32AudioDriver::GetStreamTime() {
	return streamTime_ ;
} ;

void W32AudioDriver::OnChunkDone(W32SoundBuffer *sb) {

	streamTime_+= double(sb->wavHeader_->dwBufferLength)/44100.0 ;
	sendNextChunk() ;
	clearPlayedChunk(sb) ;

	if (ticksBeforeMidi_>0) {
		ticksBeforeMidi_-- ;
	} else {
		MidiService::GetInstance()->Flush() ;
	}

} ;


void W32AudioDriver::sendNextChunk(bool notify) {

	MMRESULT res;

	if (isPlaying_) {

        AudioBufferData *abd=&(pool_[poolPlayPosition_]) ;
		assert(abd) ;
        W32SoundBuffer *sbuffer=(W32SoundBuffer *)abd->driverData_ ;
		assert(abd->driverData_) ;
        
        sbuffer->wavHeader_->lpData=(LPSTR)abd->buffer_ ;
        sbuffer->wavHeader_->dwBytesRecorded=0 ;
	    sbuffer->wavHeader_->dwFlags=0 ;
	    sbuffer->wavHeader_->lpNext=0 ;
	    sbuffer->wavHeader_->reserved=0 ;
	    sbuffer->wavHeader_->dwUser=(DWORD)sbuffer ;
        sbuffer->wavHeader_->dwBufferLength=abd->size_ ;
        sbuffer->index_=poolPlayPosition_ ;

 	    res = waveOutPrepareHeader(waveOut_, sbuffer->wavHeader_, sizeof(WAVEHDR)) ;
        if (res!=MMSYSERR_NOERROR) {                          
            Trace::Error("Failed to prepare header %d",sbuffer->index_) ;
       }
//        Trace::Debug("sending %d (%x)",sbuffer->index_,pool_[sbuffer->index_].buffer_) ;
        
//    Trace::Dump("Sending chunk %x",sbuffer->buffer_) ;
         res = waveOutWrite(waveOut_, sbuffer->wavHeader_, sizeof(WAVEHDR));

        if (res!=MMSYSERR_NOERROR) {                          
            Trace::Error("Failed to play header %d",sbuffer->index_) ;
        }    

//		Trace::Dump("sent %x",sbuffer->buffer_) ; 

        poolPlayPosition_=(poolPlayPosition_+1)%SOUND_BUFFER_COUNT ;
        
    }

    if (notify) OnNewBufferNeeded() ;
    	
//	Trace::Debug("Done") ;
} ;

void W32AudioDriver::clearPlayedChunk(W32SoundBuffer *sb) {
    
	if (isPlaying_) {
		SAFE_FREE(pool_[sb->index_].buffer_) ;
//        Trace::Debug("freed %d (%x)",sb->index_,pool_[sb->index_].buffer_) ;
		waveOutUnprepareHeader(waveOut_,sb->wavHeader_, sizeof(WAVEHDR)) ;
	}

} ;
