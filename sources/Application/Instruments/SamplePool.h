/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_POOL_H_
#define _SAMPLE_POOL_H_

#include "Application/Model/Song.h"
#include "Application/Persistency/PersistencyService.h"
#include "Foundation/Observable.h"
#include "Foundation/T_Singleton.h"
#include "WavFile.h"

#define MAX_SAMPLES MAX_SAMPLEINSTRUMENT_COUNT * 4

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
  SoundSource *GetSource(uint32_t i);
  char **GetNameList();
  int GetNameListSize();
  uint32_t FindSampleIndexByName(
      const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> &name);
  int ImportSample(const char *name, const char *projectName);
  void PurgeSample(int i, const char *projectName);
  virtual bool CheckSampleFits(int sampleSize) = 0;
  virtual uint32_t GetAvailableSampleStorageSpace() = 0;
  virtual bool unloadSample(uint32_t i) = 0;
  int8_t ReloadSample(uint8_t index, const char *name);

protected:
  virtual bool loadSample(const char *name) = 0;
  bool loadSoundFont(const char *path);
  uint32_t count_;
  char nameStore_[MAX_SAMPLES][MAX_INSTRUMENT_FILENAME_LENGTH + 1];
  char *names_[MAX_SAMPLES];
  WavFile wav_[MAX_SAMPLES];
  void swapEntries(int src, int dst);

private:
  etl::vector<I_Observer *, MAX_SAMPLEINSTRUMENT_COUNT> observers_;
};

#endif
