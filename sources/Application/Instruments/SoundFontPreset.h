#ifndef _SOUNDFONT_PRESET_H_
#define _SOUNDFONT_PRESET_H_

#include "SoundSource.h"
#include "Externals/Soundfont/SFNAV.H"
#include "Externals/Soundfont/ENAB.H"

class SoundFontPreset: public SoundSource {
public:
	SoundFontPreset(int sfID,int presetID) ;
	virtual ~SoundFontPreset() ;
	virtual int GetSize(int note) ;
	virtual int GetSampleRate(int note) ;
	virtual int GetChannelCount(int note) ;
	virtual void *GetSampleBuffer(int note) ;
	virtual bool IsMulti() ;
	virtual int GetRootNote(int note) ;
	virtual int GetLoopStart(int note) ;
	virtual int GetLoopEnd(int note) ;

	bool IsLooped(int loop) ;

protected:
	
	void checkNote(int note) ;
private:
  int sfID_;
  int presetID_;
  sfData *vect_;
  int lastNote_;
  SoundFontNavigator navigator_;
} ;
#endif
