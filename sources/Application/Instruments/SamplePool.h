
#ifndef _SAMPLE_POOL_H_
#define _SAMPLE_POOL_H_

#include "Application/Model/Song.h"
#include "Foundation/Observable.h"
#include "Foundation/T_Singleton.h"
#include "WavFile.h"

#define MAX_PIG_SAMPLES MAX_SAMPLEINSTRUMENT_COUNT

#define PROJECT_SAMPLES_DIR "samples"

enum SamplePoolEventType { SPET_INSERT, SPET_DELETE };

struct SamplePoolEvent : public I_ObservableData {
  SamplePoolEventType type_;
  int index_;
};

class SamplePool : public T_Singleton<SamplePool>, public Observable {
public:
  void Load();
  SamplePool();
  void Reset();
  ~SamplePool();
  SoundSource *GetSource(int i);
  char **GetNameList();
  int GetNameListSize();
  int ImportSample(char *name);
  void PurgeSample(int i);
  void SetProjectName(const char *name) { strcpy(projectName_, name); }

protected:
  bool loadSample(const char *path);
  bool loadSoundFont(const char *path);
  int count_;
  char *names_[MAX_PIG_SAMPLES];
  SoundSource *wav_[MAX_PIG_SAMPLES];

private:
  char projectName_[64];

#ifdef LOAD_IN_FLASH
  static int flashEraseOffset_;
  static int flashWriteOffset_;
  static int flashLimit_;
#endif
};

#endif
