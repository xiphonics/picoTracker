#ifndef _SOUND_SOURCE_H_
#define _SOUND_SOURCE_H_

class SoundSource {
public:
	SoundSource() {} ;
	virtual ~SoundSource() {} ;
	virtual int GetLoopStart(int note) { return -1 ; } ;
	virtual int GetLoopEnd(int note) { return -1 ; } ;
	virtual int GetSize(int note)=0 ;
	virtual int GetSampleRate(int note)=0 ;
	virtual int GetChannelCount(int note)=0 ;
	virtual void *GetSampleBuffer(int note)=0 ;
	virtual bool IsMulti()=0 ;
	virtual int GetRootNote(int note)=0 ;
} ;

#endif
