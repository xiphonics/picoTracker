#ifndef _SOUND_FONT_SAMPLE_H_
#define _SOUND_FONT_SAMPLE_H_

#include "SoundSource.h"
#include "System/FileSystem/FileSystem.h"
#include "Externals/Soundfont/ENAB.H"

class SoundFontSample: public SoundSource {
public:
	// From,to in bytes
	SoundFontSample(sfSampleHdr &header) ;
	virtual ~SoundFontSample() ;
	virtual int GetSize(int note) ;
	virtual int GetSampleRate(int note) ;
	virtual int GetChannelCount(int note) ;
	virtual void *GetSampleBuffer(int note) ;
	virtual bool IsMulti() {return false ; } ;
	virtual int GetRootNote(int note) ;
	virtual int GetLoopStart(int note) ;
	virtual int GetLoopEnd(int note) ;

private:
	sfSampleHdr sfHeader_ ;
} ;

#endif
