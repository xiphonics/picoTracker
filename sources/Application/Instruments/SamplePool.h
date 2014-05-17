
#ifndef _SAMPLE_POOL_H_
#define _SAMPLE_POOL_H_

#include "Foundation/T_Singleton.h"
#include "WavFile.h"
#include "Application/Model/Song.h"
#include "Foundation/Observable.h"

#define MAX_PIG_SAMPLES MAX_SAMPLEINSTRUMENT_COUNT

enum SamplePoolEventType {
	SPET_INSERT,
	SPET_DELETE
} ;

struct SamplePoolEvent: public I_ObservableData {
	SamplePoolEventType type_ ;
	int index_ ;
} ;

class SamplePool: public T_Singleton<SamplePool>,public Observable {
public:
	void Load() ;
	SamplePool() ;
	void Reset() ;
	~SamplePool() ;
	SoundSource *GetSource(int i) ;
	char **GetNameList() ;
	int GetNameListSize();
	int ImportSample(Path &path) ;
	void PurgeSample(int i) ;
	const char *GetSampleLib() ;
protected:
	bool loadSample(const char * path) ;
	bool loadSoundFont(const char *path);
	int count_ ;
	char* names_[MAX_PIG_SAMPLES] ;
	SoundSource *wav_[MAX_PIG_SAMPLES] ;
} ;

#endif
