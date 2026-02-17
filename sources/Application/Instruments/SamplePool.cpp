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
#include "Externals/SRC/common.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/string_stream.h"
#include "Foundation/Constants/SpecialCharacters.h"
#include "Foundation/Services/MemoryService.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "System/FileSystem/I_File.h"
#include "System/io/Status.h"
#include "WavHeader.h"
#include <cstdint>
#include <stdlib.h>
#include <string.h>
#include <utility>

SamplePool::SamplePool() : Observable(&observers_) {
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

void SamplePool::Load(const char *projectName) {
  auto fs = FileSystem::GetInstance();
  if (!fs->chdir(PROJECTS_DIR) || !fs->chdir(projectName) ||
      !fs->chdir(PROJECT_SAMPLES_DIR)) {
    Trace::Error("Failed to chdir into %s/%s/%s", PROJECTS_DIR, projectName,
                 PROJECT_SAMPLES_DIR);
  }
  // First, find all wav files
  fs->list(&MemoryPool::fileIndexList, ".wav", false);
  char name[PFILENAME_SIZE];
  uint totalSamples = MemoryPool::fileIndexList.size();
  for (uint i = 0; i < totalSamples; i++) {
    fs->getFileName(MemoryPool::fileIndexList[i], name, PFILENAME_SIZE);
    if (fs->getFileType(MemoryPool::fileIndexList[i]) == PFT_FILE) {
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
      int prog10 = progress / 10;

      char progressBar[13];
      for (int j = 1; j < 11; j++) {
        progressBar[j] = j >= prog10 ? char_battery_empty : char_block_full;
      }
      progressBar[0] = char_button_border_left;
      progressBar[11] = char_button_border_right;
      progressBar[12] = 0;

      Status::SetMultiLine("Copying %s" char_indicator_ellipsis_s
                           "\n \n%s %d%%",
                           name, (const char *)progressBar, progress);

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

    uint32_t progress = 100;
    if (totalSize > 0) {
      progress = (totalRead * 100) / totalSize;
    }
    Status::SetMultiLine("Loading:\n%s\n%d%%", projSampleFilename.c_str(),
                         progress);
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
  }

  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = count_ - 1;
  ev.type_ = SPET_INSERT;
  NotifyObservers(&ev);
  return status ? (count_ - 1) : -1;
};

void SamplePool::PurgeSample(int i, const char *projectName) {
  auto fs = FileSystem::GetInstance();

  etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> buffer;
  etl::string_stream delPath(buffer);

  delPath << "/" << PROJECTS_DIR << "/" << projectName << "/"
          << PROJECT_SAMPLES_DIR << "/" << names_[i];

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

  // now notify observers
  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = i;
  ev.type_ = SPET_DELETE;
  NotifyObservers(&ev);
};

// returns the new samples index or -1 on error
int8_t SamplePool::ReloadSample(uint8_t index, const char *name) {
  if (unloadSample(index)) {
    if (loadSample(name)) {
      return count_ - 1;
    }
  }
  return -1;
}

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
