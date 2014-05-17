
#include "W32Audio.h"
#include "W32AudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"
#include "System/Console/Trace.h"

W32Audio::W32Audio(AudioSettings &settings):Audio(settings),
	sampleRate_(44100) 
{
	index_=WAVE_MAPPER ;
	Trace::Log("AUDIO","Using MMSYSTEM api") ;
	const char *deviceName=settings.audioDevice_.c_str() ;
	if (strlen(deviceName)!=0) {
		unsigned int numdev=waveOutGetNumDevs() ;
		for (unsigned int i=0;i<numdev;i++) {
			WAVEOUTCAPS caps;
			waveOutGetDevCaps(i,&caps,sizeof(caps)) ;
			if (!strncmp(deviceName,caps.szPname,strlen(deviceName))) {
				Trace::Log("AUDIO","Selecting audio device: %s",caps.szPname) ;
				index_=i ;
				break ;
			}
		} ;
	} ;
}

W32Audio::~W32Audio() {
}

void W32Audio::Init() {
     W32AudioDriver *drv=new W32AudioDriver(index_,settings_) ;
     AudioOutDriver *out=new AudioOutDriver(*drv) ;
     Insert(out) ;
	 sampleRate_=drv->GetSampleRate() ;
} ;

void W32Audio::Close() {
     IteratorPtr<AudioOut>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         AudioOut &current=it->CurrentItem() ;
         current.Close() ;
     }
} ;

int W32Audio::GetSampleRate()  {
	return sampleRate_ ;
} ;