#ifndef _AUDIO_MIXER_H_
#define _AUDIO_MIXER_H_

#include "Application/Instruments/WavFileWriter.h"
#include "AudioModule.h"
#include "Foundation/T_SimpleList.h"
#include "Services/Audio/AudioDriver.h" // for MAX_SAMPLE_COUNT
#include <string>

class AudioMixer: public AudioModule,public T_SimpleList<AudioModule> {
public:
	AudioMixer(const char *name) ;
	virtual ~AudioMixer() ;
	virtual bool Render(fixed *buffer,int samplecount) ;
	void SetFileRenderer(const char *path) ;
	void EnableRendering(bool enable) ;
	void SetVolume(fixed volume) ;
private:
	bool enableRendering_ ;
	std::string renderPath_ ;
	WavFileWriter *writer_ ;
	fixed volume_ ;
	std::string name_ ;

  static fixed renderBuffer_[MAX_SAMPLE_COUNT * 2];
} ;
#endif
