#ifndef _AUDIO_FILE_STREAMER_H_
#define _AUDIO_FILE_STREAMER_H_

#include "Services/Audio/AudioModule.h"
#include "System/FileSystem/FileSystem.h"
#include "Application/Instruments/WavFile.h"

enum AudioFileStreamerMode {
	AFSM_STOPPED,
	AFSM_PLAYING
} ;

class AudioFileStreamer: public AudioModule {
public:
	AudioFileStreamer() ;
	virtual ~AudioFileStreamer() ;
	virtual bool Render(fixed *buffer,int samplecount) ;
	bool Start(const Path &) ;
	void Stop() ;
protected:
	AudioFileStreamerMode mode_ ;
	Path path_ ;
	bool newPath_ ;
	WavFile *wav_ ;
	int position_ ;
	int shift_ ;
} ;

#endif 
