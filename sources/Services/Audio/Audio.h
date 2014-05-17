
#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "Foundation/T_Factory.h"
#include "Foundation/T_SimpleList.h"
#include "AudioOut.h"
#include "AudioSettings.h"


class Audio:public T_Factory<Audio>,public T_SimpleList<AudioOut> {
public:
	Audio(AudioSettings &settings) ;
	virtual ~Audio() ;
	virtual void Init()=0 ;
	virtual void Close()=0 ;
	virtual int GetSampleRate() {return 44100 ; } ;
	virtual int GetMixerVolume() { return 100 ; } ;
	virtual void SetMixerVolume(int volume) {} ;

	const char *GetAudioAPI() ;
	const char *GetAudioDevice() ;
	int GetAudioBufferSize() ;
	int GetAudioPreBufferCount() ;

protected:
	AudioSettings settings_ ;

private:
	std::string audioAPI_ ;
	std::string audioDevice_ ;
	int audioBufferSize_ ;
	int preBufferCount_ ;
} ;
#endif
