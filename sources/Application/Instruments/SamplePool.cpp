/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SamplePool.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Utils/DrawUtils.h"
#include "Externals/SRC/common.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/string_stream.h"
#include "Foundation/Constants/SpecialCharacters.h"
#include "SampleCacheValidate.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "System/FileSystem/I_File.h"
#include "System/io/Status.h"
#include "WavHeader.h"
#include <cstdint>
#include <stdlib.h>
#include <string.h>
#include <utility>

SamplePool::SamplePool()
    : Observable(&observers_), sampleCacheStale_(false),
      bulkCacheUpdateDepth_(0) {
  count_ = 0;
  for (int i = 0; i < MAX_SAMPLES; i++) {
    names_[i] = nameStore_[i];
    nameStore_[i][0] = '\0';
  };
};

SamplePool::~SamplePool() {
  for (int i = 0; i < MAX_SAMPLES; i++) {
    wav_[i].Close();
  };
};

void SamplePool::updateStatus(uint32_t index, uint32_t total,
                              const char *message) {
  progressBar_t progressBar;
  uint32_t percentage = (total > 0) ? (index * 100U) / total : 100U;
  fillProgressBar(index, total, &progressBar);
  Status::SetMultiLine("%s %.19s" char_indicator_ellipsis_s " \n \n %s %3d%%",
                       message, importName, progressBar,
                       static_cast<int>(percentage));
};

etl::vector<SampleCacheEntry, MAX_SAMPLES> &SamplePool::cacheEntryScratch() {
  // Initialised on first use, so it costs nothing at start-up and does not take
  // part in static initialisation order.
  static etl::vector<SampleCacheEntry, MAX_SAMPLES> entries;
  return entries;
}

void SamplePool::InvalidateSampleCache() {
  sampleCacheStale_ = true;
  if (!PersistencyService::GetInstance()->DeleteSampleCache()) {
    // Still stale: the flag alone is enough to keep later writes from
    // republishing state that no longer matches the WAVs on the card.
    Trace::Error("SAMPLEPOOL: CACHE INVALIDATED (file could not be deleted); "
                 "no cache will be written until project reload");
    return;
  }
  Trace::Log("SAMPLEPOOL", "CACHE INVALIDATED - no cache write until the "
                           "project is reloaded");
}

void SamplePool::DiscardSampleCache() {
  if (!PersistencyService::GetInstance()->DeleteSampleCache()) {
    Trace::Error("SAMPLEPOOL: write-ahead cache delete failed");
    return;
  }
  Trace::Log("SAMPLEPOOL",
             "CACHE DISCARDED ahead of rewriting flash or the project WAVs");
}

void SamplePool::RekeySampleCache(const char *projectName) {
  // Goes through the same gate as every other write, so a pool that has
  // diverged from its WAVs is never filed under the new name either.
  SaveSampleCacheForCurrentPool(projectName, false);
}

void SamplePool::BeginBulkCacheUpdate() { bulkCacheUpdateDepth_++; }

void SamplePool::EndBulkCacheUpdate(const char *projectName) {
  if (bulkCacheUpdateDepth_ > 0) {
    bulkCacheUpdateDepth_--;
  }
  if (bulkCacheUpdateDepth_ == 0) {
    SaveSampleCacheForCurrentPool(projectName, false);
  }
}

void SamplePool::SaveSampleCacheForCurrentPool(const char *projectName,
                                               bool verify) {
  if (sampleCacheStale_) {
    Trace::Log("SAMPLEPOOL", "Sample cache stale - skipping write for '%s'",
               projectName);
    return;
  }
  if (bulkCacheUpdateDepth_ > 0) {
    // Deferred to EndBulkCacheUpdate(): a mid-batch cache would not describe
    // the operation as a whole.
    return;
  }
  writeSampleCache(projectName, verify);
}

bool SamplePool::validateCacheAgainstSd(
    const char *projectName, const etl::ivector<SampleCacheEntry> &entries) {
  auto fs = FileSystem::GetInstance();
  if (!fs->chdir(PROJECTS_DIR) || !fs->chdir(projectName) ||
      !fs->chdir(PROJECT_SAMPLES_DIR)) {
    Trace::Log("SAMPLEPOOL",
               "Sample cache unverified for '%s': samples dir unavailable",
               projectName);
    return false;
  }

  // Directory scan plus one stat per candidate; no sample data is read, and no
  // per-sample storage is kept, so this stays cheap on RAM.
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexes;
  fs->list(&fileIndexes, ".wav", false);

  size_t cardSamples = 0;
  size_t cardSamplesUnchanged = 0;
  char name[PFILENAME_SIZE];
  for (size_t j = 0; j < fileIndexes.size(); ++j) {
    fs->getFileName(fileIndexes[j], name, PFILENAME_SIZE);
    // Skip exactly what Load() would skip so the counts stay comparable.
    if (fs->getFileType(fileIndexes[j]) != PFT_FILE ||
        strlen(name) > MAX_INSTRUMENT_FILENAME_LENGTH) {
      continue;
    }
    cardSamples++;
    auto pairing = pairCardSampleWithCache(
        name, static_cast<uint32_t>(fs->getFileSize(fileIndexes[j])),
        entries.data(), entries.size());
    if (pairing == SampleCachePairing::Matched) {
      cardSamplesUnchanged++;
    }
  }

  if (!cacheAndCardAgree(cardSamples, cardSamplesUnchanged, entries.size())) {
    Trace::Log("SAMPLEPOOL",
               "Sample cache stale for '%s': card has %u sample(s), %u "
               "unchanged, cache holds %u",
               projectName, (unsigned)cardSamples,
               (unsigned)cardSamplesUnchanged, (unsigned)entries.size());
    return false;
  }
  return true;
}

bool SamplePool::LoadFromCache(const char *projectName) {
  if (sampleCacheStale_) {
    // Pool contents no longer match the WAVs on SD and the on-disk cache was
    // already removed; go to SD and republish.
    Trace::Log("SAMPLEPOOL", "Sample cache invalidated - SD load");
    return false;
  }
  auto &entries = cacheEntryScratch();
  entries.clear();
  uint32_t eraseOff = 0;
  uint32_t writeOff = 0;
  auto *ps = PersistencyService::GetInstance();
  auto res = ps->LoadSampleCache(projectName, GetSampleCacheBuildId(), entries,
                                 eraseOff, writeOff);
  if (res != PERSIST_LOADED) {
    Trace::Log("SAMPLEPOOL",
               "No usable sample cache for '%s' (res=%d) — SD load",
               projectName, (int)res);
    return false;
  }
  // Monotonicity check: write offset must cover every cached entry. Use
  // subtraction so a corrupt offset/size pair cannot wrap past writeOff.
  for (size_t i = 0; i < entries.size(); ++i) {
    const SampleCacheEntry &entry = entries[i];
    if (entry.flashOffset > writeOff ||
        entry.sampleBufferSize > writeOff - entry.flashOffset) {
      Trace::Error("SAMPLEPOOL: cache entry '%s' exceeds writeOff", entry.name);
      ps->DeleteSampleCache();
      return false;
    }
  }
  if (!ValidateSampleCache(entries, eraseOff, writeOff)) {
    Trace::Error("SAMPLEPOOL: cache has invalid flash allocator state");
    ps->DeleteSampleCache();
    return false;
  }
  // The cache only describes audio that is still byte-for-byte what was loaded
  // into flash, so check the source files before trusting it.
  if (!validateCacheAgainstSd(projectName, entries)) {
    ps->DeleteSampleCache();
    return false;
  }
  ResumeFromCache(eraseOff, writeOff);
  for (size_t i = 0; i < entries.size(); ++i) {
    if (!rebuildSampleFromCache(entries[i])) {
      Trace::Error("SAMPLEPOOL: failed to rebuild '%s' from cache",
                   entries[i].name);
      Reset();
      ps->DeleteSampleCache();
      return false;
    }
  }
  // validateCacheAgainstSd() above left the cwd in /projects/<name>/samples, so
  // a cache hit ends up where the SD path below always left it. Views set their
  // own cwd before listing (goProjectSamplesDir, ImportView) but this keeps the
  // two paths indistinguishable either way.
  Trace::Log("SAMPLEPOOL", "Loaded %u samples from cache for '%s'",
             (unsigned)entries.size(), projectName);
  return true;
}

void SamplePool::Load(const char *projectName) {
  // Either branch below leaves the pool exactly matching the cached flash
  // state or the SD contents, so the stale flag no longer applies.
  sampleCacheStale_ = false;
  if (LoadFromCache(projectName)) {
    return;
  }
  auto fs = FileSystem::GetInstance();
  if (!fs->chdir(PROJECTS_DIR) || !fs->chdir(projectName) ||
      !fs->chdir(PROJECT_SAMPLES_DIR)) {
    Trace::Error("Failed to chdir into %s/%s/%s", PROJECTS_DIR, projectName,
                 PROJECT_SAMPLES_DIR);
  }
  // Write-ahead invalidation: from the first loadSample() on we rewrite flash
  // destructively while /.current still names this project. Deleting the cache
  // up front means a power cut part way through leaves a cold SD load rather
  // than a cache hit pointing at half-written flash.
  DiscardSampleCache();

  // First, find all wav files
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexes;
  fs->list(&fileIndexes, ".wav", false);
  char name[PFILENAME_SIZE];
  uint totalSamples = fileIndexes.size();

  // store for ui updates
  importCount = totalSamples;

  for (uint i = 0; i < totalSamples; i++) {
    importIndex = i;
    importName = name;

    fs->getFileName(fileIndexes[i], name, PFILENAME_SIZE);
    if (fs->getFileType(fileIndexes[i]) == PFT_FILE) {
      // Check if the filename exceeds the maximum allowed length
      if (strlen(name) > MAX_INSTRUMENT_FILENAME_LENGTH) {
        Trace::Error(
            "SAMPLEPOOL: Sample filename exceeds maximum length: %s (%zu > %d)",
            name, strlen(name), MAX_INSTRUMENT_FILENAME_LENGTH);
        // Skip this sample and continue with the next one
        continue;
      }

      // Show progress as percentage
      int progress = (int)((i * 100) / totalSamples);

      updateStatus(importIndex, importCount, "Loading");
      loadSample(name);
    }
    if (i == MAX_SAMPLES) {
      Trace::Error("Warning maximum sample count reached");
      break;
    };
  };

  // now sort the samples
  int rest = count_;
  while (rest > 0) {
    int index = 0;
    for (int i = 1; i < rest; i++) {
      if (strcmp(names_[i], names_[index]) > 0) {
        index = i;
      };
    };
    swapEntries(index, rest - 1);
    rest--;
  };

  // Write sample cache so that next boot can skip SD reloads. This is the one
  // place the read-back check pays for itself: everything else about the pool
  // has just been rebuilt, and a cache that silently failed to land would be
  // indistinguishable from a working one.
  SaveSampleCacheForCurrentPool(projectName, true);
};

void SamplePool::RebuildCacheFromSd(const char *projectName) {
  // Reset first: this is what releases the pool and rewinds the flash allocator
  // to the start of the sample area so the reload packs tightly.
  Reset();
  // Force Load() down its SD path rather than accepting the very cache we are
  // replacing. Load() clears the stale flag and republishes the cache itself.
  DiscardSampleCache();
  Load(projectName);
};

SoundSource *SamplePool::GetSource(uint32_t i) {
  if (i < 0 || i >= count_) {
    return nullptr;
  }
  return &wav_[i];
};

char **SamplePool::GetNameList() { return names_; };

int SamplePool::GetNameListSize() { return count_; };

uint32_t SamplePool::FindSampleIndexByName(
    const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> &name) {
  char **names = GetNameList();
  uint16_t count = GetNameListSize();
  for (uint16_t i = 0; i < count; ++i) {
    if (names[i] && strcmp(names[i], name.c_str()) == 0) {
      return i;
    }
  }
  return -1;
}

#define IMPORT_CHUNK_SIZE 512
static constexpr int32_t kImportInputSamples =
    IMPORT_CHUNK_SIZE / static_cast<int32_t>(sizeof(int16_t));
static constexpr int32_t kImportMaxOutputSamples =
    (kImportInputSamples * SRC_MAX_RATIO) + 8;
static float importResampleIn_[kImportInputSamples];
static float importResampleOut_[kImportMaxOutputSamples];
static int16_t importResampleOutInt16_[kImportMaxOutputSamples];

int SamplePool::ImportSample(const char *name, const char *projectName) {
  if (count_ == MAX_SAMPLES) {
    return -1;
  }

  WavFile wav;
  auto wavRes = wav.Open(name);
  if (!wavRes) {
    Trace::Error("Failed to open sample input file:%s", name);
    return -1;
  }

  // will truncate too long filenames to make sure the filename imported into
  // the project is with filename length limit
  etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> projSampleFilename(name);
  if (projSampleFilename.is_truncated()) {
    // Truncate the string in-place and then append the extension
    projSampleFilename =
        projSampleFilename.substr(0, MAX_INSTRUMENT_FILENAME_LENGTH - 4);
    projSampleFilename.append(".wav");
  }

  etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> projectSamplePath("/projects/");
  projectSamplePath.append(projectName);
  projectSamplePath.append("/samples/");
  projectSamplePath.append(projSampleFilename);
  Status::SetMultiLine("Loading %s->\n%s", name, projSampleFilename);

  auto fout = FileSystem::GetInstance()->Open(projectSamplePath.c_str(), "w");
  if (!fout) {
    Trace::Error("Failed to open sample project file:%s", projectSamplePath);
    return -1;
  };

  const int32_t sourceSampleRate = wav.GetSampleRate(-1);
  const int32_t channelCount = wav.GetChannelCount(-1);
  const int32_t importResampler =
      Config::GetInstance()->GetValue("IMPORTRESAMP");
  const bool shouldResample =
      (importResampler > 0) && (sourceSampleRate != 44100);
  const int32_t outputSampleRate = shouldResample ? 44100 : sourceSampleRate;

  if (!WavHeaderWriter::WriteHeader(fout.get(), outputSampleRate, channelCount,
                                    16)) {
    Trace::Error("Failed to write WAV header for:%s", projectSamplePath);
    return -1;
  }

  // copy file to current project as 16-bit PCM
  uint8_t buffer[IMPORT_CHUNK_SIZE];
  uint32_t bytesRead = 0;
  uint32_t samplesRead = 0;
  uint32_t totalRead = 0;
  uint32_t totalWrittenFrames = 0;
  uint32_t totalSize = wav.GetDiskSize(-1);

  importName = name;

  wav.Rewind();
  SRC_STATE *resampler = nullptr;
  if (shouldResample) {
    int32_t converterType = SRC_LINEAR;
    if (importResampler == 2) {
      converterType = SRC_SINC_FASTEST;
    } else if (importResampler == 3) {
      converterType = SRC_SINC_MEDIUM_QUALITY;
    }

    int srcError = 0;
    resampler = src_new(converterType, channelCount, &srcError);
    if (!resampler || srcError != SRC_ERR_NO_ERROR) {
      Trace::Error("Failed to initialize resampler (%d)", srcError);
      return -1;
    }
    src_reset(resampler);
  }

  const double srcRatio =
      shouldResample ? (44100.0 / static_cast<double>(sourceSampleRate)) : 1.0;

  while (true) {
    if (!shouldResample) {
      if (!wav.Read(buffer, sizeof(buffer), &bytesRead)) {
        Trace::Error("Failed reading sample data from:%s", name);
        return -1;
      }
      if (bytesRead == 0) {
        break;
      }
      totalRead += bytesRead;
      uint32_t written = fout->Write(buffer, 1, bytesRead);
      if (written != bytesRead) {
        Trace::Error("Failed writing sample data to:%s", projectSamplePath);
        return -1;
      }
      totalWrittenFrames +=
          bytesRead / (static_cast<uint32_t>(channelCount) * 2);
    } else {
      if (!wav.ReadFloat(importResampleIn_, kImportInputSamples,
                         &samplesRead)) {
        Trace::Error("Failed reading sample data from:%s", name);
        return -1;
      }
      if (samplesRead == 0) {
        break;
      }
      totalRead += samplesRead * 2;

      const uint32_t inputFrames =
          samplesRead / static_cast<uint32_t>(channelCount);
      uint32_t framesRemaining = inputFrames;
      float *inPtr = importResampleIn_;
      while (framesRemaining > 0) {
        SRC_DATA data;
        memset(&data, 0, sizeof(data));
        data.data_in = inPtr;
        data.input_frames = static_cast<long>(framesRemaining);
        data.data_out = importResampleOut_;
        data.output_frames = static_cast<long>(
            kImportMaxOutputSamples / static_cast<int32_t>(channelCount));
        data.src_ratio = srcRatio;
        data.end_of_input = 0;

        int err = src_process(resampler, &data);
        if (err != SRC_ERR_NO_ERROR) {
          Trace::Error("Resample failed: %s", src_strerror(err));
          return -1;
        }

        if (data.output_frames_gen > 0) {
          const int32_t outputSamples =
              static_cast<int32_t>(data.output_frames_gen) * channelCount;
          src_float_to_short_array(importResampleOut_, importResampleOutInt16_,
                                   outputSamples);
          const int32_t bytesToWrite = outputSamples * sizeof(int16_t);
          uint32_t written =
              fout->Write(importResampleOutInt16_, 1, bytesToWrite);
          if (written != static_cast<uint32_t>(bytesToWrite)) {
            Trace::Error("Failed writing sample data to:%s", projectSamplePath);
            return -1;
          }
          totalWrittenFrames += data.output_frames_gen;
        }

        framesRemaining -= data.input_frames_used;
        inPtr += data.input_frames_used * channelCount;
      }
    }

    uint32_t total = totalSize * 2U;

    importCount = total;
    importIndex = totalRead;
    updateStatus(totalRead, total, "Copying");
  }

  // Flush the resampler to write any delayed tail samples after input ends.
  if (shouldResample && resampler) {
    while (true) {
      SRC_DATA data;
      memset(&data, 0, sizeof(data));
      data.data_in = nullptr;
      data.input_frames = 0;
      data.data_out = importResampleOut_;
      data.output_frames = static_cast<long>(
          kImportMaxOutputSamples / static_cast<int32_t>(channelCount));
      data.src_ratio = srcRatio;
      data.end_of_input = 1;

      int err = src_process(resampler, &data);
      if (err != SRC_ERR_NO_ERROR) {
        Trace::Error("Resample flush failed: %s", src_strerror(err));
        return -1;
      }

      if (data.output_frames_gen <= 0) {
        break;
      }

      const int32_t outputSamples =
          static_cast<int32_t>(data.output_frames_gen) * channelCount;
      src_float_to_short_array(importResampleOut_, importResampleOutInt16_,
                               outputSamples);
      const int32_t bytesToWrite = outputSamples * sizeof(int16_t);
      uint32_t written = fout->Write(importResampleOutInt16_, 1, bytesToWrite);
      if (written != static_cast<uint32_t>(bytesToWrite)) {
        Trace::Error("Failed writing sample data to:%s", projectSamplePath);
        return -1;
      }
      totalWrittenFrames += data.output_frames_gen;
    }
    src_delete(resampler);
  }

  if (!WavHeaderWriter::UpdateFileSize(
          fout.get(),
          shouldResample ? totalWrittenFrames
                         : static_cast<uint32_t>(wav.GetSize(-1)),
          channelCount, 2)) {
    Trace::Error("Failed to update WAV header for:%s", projectSamplePath);
    return -1;
  }

  // Close the output file before re-opening it for import.
  fout.reset();

  // Write-ahead invalidation: loadSample() appends to flash, so from here on a
  // power cut would leave the cache pointing at a partly written pool.
  DiscardSampleCache();

  // now load the sample into memory/flash from the project pool path
  bool status = loadSample(projectSamplePath.c_str());
  if (status) {
    // Replace stored name with truncated filename so matches the potentially
    // truncated filename we actually stored into the project pool subdir
    const int loadedIndex = count_ - 1;
    if (loadedIndex >= 0) {
      projSampleFilename.copy(nameStore_[loadedIndex],
                              projSampleFilename.size());
      nameStore_[loadedIndex][projSampleFilename.size()] = '\0';
    }
    SaveSampleCacheForCurrentPool(projectName, false);
  }

  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = count_ - 1;
  ev.type_ = SPET_INSERT;
  NotifyObservers(&ev);
  return status ? (count_ - 1) : -1;
};

// Register a WAV that already sits in the current project's samples subdir as
// a new pool entry. Unlike ImportSample() there is nothing to convert or
// resample, the sample editor wrote the file itself. The entry is appended and
// unsorted, exactly as ImportSample() leaves it, and flash can only be
// appended to, so callers must make sure the name is not already pooled.
// Returns the new pool index or -1.
int SamplePool::LoadProjectSample(const char *name) {
  if (count_ >= MAX_SAMPLES) {
    return -1;
  }

  // loadSample() reports progress through these. There is a single file and it
  // is already on the card, so show it as done instead of reusing whatever the
  // last project load or import left behind.
  importName = name;
  importIndex = 1;
  importCount = 1;

  if (!loadSample(name)) {
    return -1;
  }

  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = count_ - 1;
  ev.type_ = SPET_INSERT;
  NotifyObservers(&ev);
  return count_ - 1;
}

void SamplePool::PurgeSample(int i, const char *projectName) {
  auto fs = FileSystem::GetInstance();

  etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> buffer;
  etl::string_stream delPath(buffer);

  delPath << "/" << PROJECTS_DIR << "/" << projectName << "/"
          << PROJECT_SAMPLES_DIR << "/" << names_[i];

  // Write-ahead invalidation: the WAV disappears from SD before the cache can
  // be rewritten, so delete first and let the write at the end republish.
  DiscardSampleCache();

  // delete file
  FileSystem::GetInstance()->DeleteFile(delPath.str().c_str());
  // shift all entries from deleted to end
  for (uint32_t j = i; j < count_ - 1; j++) {
    wav_[j] = std::move(wav_[j + 1]);
    memcpy(nameStore_[j], nameStore_[j + 1],
           MAX_INSTRUMENT_FILENAME_LENGTH + 1);
  };
  // decrease sample count
  count_--;
  wav_[count_].Close();
  nameStore_[count_][0] = '\0';

  SaveSampleCacheForCurrentPool(projectName, false);

  // now notify observers
  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = i;
  ev.type_ = SPET_DELETE;
  NotifyObservers(&ev);
};

void SamplePool::swapEntries(int src, int dst) {
  if (src == dst) {
    return;
  }
  std::swap(wav_[src], wav_[dst]);
  char tmp[MAX_INSTRUMENT_FILENAME_LENGTH + 1];
  memcpy(tmp, nameStore_[src], sizeof(tmp));
  memcpy(nameStore_[src], nameStore_[dst], sizeof(tmp));
  memcpy(nameStore_[dst], tmp, sizeof(tmp));
}
