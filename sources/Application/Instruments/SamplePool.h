
#ifndef _SAMPLE_POOL_H_
#define _SAMPLE_POOL_H_

#include "Application/Model/Song.h"
#include "Application/Persistency/PersistencyService.h"
#include "Foundation/Observable.h"
#include "Foundation/T_Singleton.h"
#include "WavFile.h"

#define MAX_SAMPLES 64

enum SamplePoolEventType { SPET_INSERT, SPET_DELETE };

struct SamplePoolEvent : public I_ObservableData {
  SamplePoolEventType type_;
  int index_;
};

class SamplePool : public T_Factory<SamplePool>, public Observable {
public:
  void Load(const char *projectName);
  SamplePool();
  virtual void Reset() = 0;
  virtual ~SamplePool();
  SoundSource *GetSource(int i);
  char **GetNameList();
  int GetNameListSize();
  int ImportSample(char *name, const char *projectName);
  void PurgeSample(int i, const char *projectName);
  virtual bool CheckSampleFits(int sampleSize) { return true; } // Default implementation always returns true

protected:
  virtual bool loadSample(const char *name) = 0;
  virtual bool unloadSample() = 0;
  bool loadSoundFont(const char *path);
  int count_;
  char *names_[MAX_SAMPLES];
  SoundSource *wav_[MAX_SAMPLES];

private:
  etl::vector<I_Observer *, MAX_SAMPLEINSTRUMENT_COUNT> observers_;
};

#endif
