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

#include "Application/Instruments/SampleCacheEntry.h"
#include "Application/Model/Song.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/vector.h"
#include "Foundation/Observable.h"
#include "Foundation/T_Singleton.h"
#include "WavFile.h"
#include <cstdint>

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

  // Marks the on-disk sample cache unusable and remembers it until the pool is
  // rebuilt by Load(). Needed whenever flash and the project WAVs diverge (a
  // sample edit, a removed sample): deleting the file alone is not enough,
  // because a later import or purge would republish a cache describing the
  // stale in-flash state and silently undo the change on next reload.
  void InvalidateSampleCache();
  bool IsSampleCacheStale() const { return sampleCacheStale_; }

  virtual bool rebuildSampleFromCache(const SampleCacheEntry &e) {
    return false;
  }
  virtual bool
  ValidateSampleCache(const etl::ivector<SampleCacheEntry> &entries,
                      uint32_t flashEraseOffset,
                      uint32_t flashWriteOffset) const {
    return true;
  }
  virtual void ResumeFromCache(uint32_t flashEraseOffset,
                               uint32_t flashWriteOffset) {}
  // Firmware-specific id mixed into the sample-cache header so a firmware
  // update whose flash layout has shifted rejects a stale cache. Default 0
  // disables the check on platforms without a meaningful build id.
  virtual uint32_t GetSampleCacheBuildId() const { return 0; }

  bool LoadFromCache(const char *projectName);

protected:
  virtual void updateStatus(uint32_t current, uint32_t total,
                            const char *message);
  virtual bool loadSample(const char *name) = 0;
  bool loadSoundFont(const char *path);
  // Single gate for publishing cache state: does nothing while the pool is
  // known to be out of sync with the WAVs on SD.
  void SaveSampleCacheForCurrentPool(const char *projectName);
  virtual void writeSampleCache(const char *projectName) {}

  uint32_t count_;
  bool sampleCacheStale_;
  char nameStore_[MAX_SAMPLES][MAX_INSTRUMENT_FILENAME_LENGTH + 1];
  char *names_[MAX_SAMPLES];
  WavFile wav_[MAX_SAMPLES];
  etl::vector<SampleCacheEntry, MAX_SAMPLES> sampleCacheEntries_;
  void swapEntries(int src, int dst);

  uint32_t importCount;
  uint32_t importIndex;
  const char *importName;

private:
  etl::vector<I_Observer *, MAX_SAMPLEINSTRUMENT_COUNT> observers_;
};

#endif
