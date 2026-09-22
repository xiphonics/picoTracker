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
  int LoadProjectSample(const char *name);
  void PurgeSample(int i, const char *projectName);
  virtual bool CheckSampleFits(int sampleSize) = 0;
  virtual uint32_t GetAvailableSampleStorageSpace() = 0;
  // Drop a sample from the pool. picoTracker cannot reclaim flash in place, so
  // the implementation currently always fails and the entry survives until the
  // project is reloaded - callers must not gate cache invalidation, or anything
  // else, on the result.
  virtual bool unloadSample(uint32_t i) = 0;

  // Marks the on-disk sample cache unusable and remembers it until the pool is
  // rebuilt by Load(). Needed whenever flash and the project WAVs diverge (a
  // sample edit, a removed sample): deleting the file alone is not enough,
  // because a later import or purge would republish a cache describing the
  // stale in-flash state and silently undo the change on next reload.
  void InvalidateSampleCache();

  // Delete the on-disk cache *without* marking the pool stale. Call right
  // before flash or the project WAVs are destructively rewritten so that a
  // power cut mid-operation leaves no cache behind to hit, rather than relying
  // on the replacement write at the end of the operation. The caller is
  // expected to republish the cache once the operation completes.
  void DiscardSampleCache();

  // File the cache under a different project name without touching flash. "Save
  // as" copies the samples byte for byte, so the cached offsets and
  // fingerprints stay correct and only the name in the header is wrong;
  // republishing it keeps the rename from costing a full sample reload on the
  // next boot. Does nothing while the pool is stale.
  void RekeySampleCache(const char *projectName);

  // Collapse a batch of imports or purges into one cache write. Nestable.
  void BeginBulkCacheUpdate();
  void EndBulkCacheUpdate(const char *projectName);

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

  // Reload every sample of the project from SD so that flash is repacked from
  // the start of the sample area, then republish the cache. Import and purge
  // only ever append, so this is the way back from a write high-water mark that
  // no longer reflects how much is really in use. Nothing may be playing: every
  // sample pointer moves, and the caller should reload the project afterwards.
  void RebuildCacheFromSd(const char *projectName);

protected:
  // Confirm the WAVs on the card still match the cached entries, using
  // directory metadata only - no sample data is read. Rejects the cache when a
  // sample was replaced, resized, added or removed out of band, since the pool
  // (and so every instrument sample index) would otherwise diverge from a
  // fresh SD load. Leaves the cwd in the project samples dir, which a full
  // Load() has always done as a side effect.
  bool validateCacheAgainstSd(const char *projectName,
                              const etl::ivector<SampleCacheEntry> &entries);

  virtual void updateStatus(uint32_t current, uint32_t total,
                            const char *message);
  virtual bool loadSample(const char *name) = 0;
  bool loadSoundFont(const char *path);
  // Single gate for publishing cache state: does nothing while the pool is
  // known to be out of sync with the WAVs on SD. verify reads the file back to
  // confirm it reached the card - worth the extra SD traffic after a full
  // project load, not after every single import or purge.
  void SaveSampleCacheForCurrentPool(const char *projectName, bool verify);
  virtual void writeSampleCache(const char *projectName, bool verify) {}

  // Staging buffer for the cache, shared by the only two places that need it:
  // LoadFromCache() and the derived writeSampleCache(). Deliberately a function
  // local static rather than a member so the 3.2 KB does not sit inside the
  // base object and inside every future subclass that does not use the cache at
  // all. One buffer, not one per user - two would cost 6.5 KB. Not reentrant:
  // only ever touched from the main thread while loading, importing or purging.
  static etl::vector<SampleCacheEntry, MAX_SAMPLES> &cacheEntryScratch();

  uint32_t count_;
  bool sampleCacheStale_;
  uint8_t bulkCacheUpdateDepth_;
  char nameStore_[MAX_SAMPLES][MAX_INSTRUMENT_FILENAME_LENGTH + 1];
  char *names_[MAX_SAMPLES];
  WavFile wav_[MAX_SAMPLES];
  void swapEntries(int src, int dst);

  uint32_t importCount;
  uint32_t importIndex;
  const char *importName;

private:
  etl::vector<I_Observer *, MAX_SAMPLEINSTRUMENT_COUNT> observers_;
};

#endif
