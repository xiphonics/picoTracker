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
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/string_stream.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "System/FileSystem/I_File.h"
#include "System/io/Status.h"
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
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexes;
  fs->list(&fileIndexes, ".wav", false);
  char name[PFILENAME_SIZE];
  uint totalSamples = fileIndexes.size();
  for (uint i = 0; i < totalSamples; i++) {
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
      Status::SetMultiLine("Copying:\n%s (%d%%)", name, progress);
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

SoundSource *SamplePool::GetSource(int i) { return &wav_[i]; };

char **SamplePool::GetNameList() { return names_; };

int SamplePool::GetNameListSize() { return count_; };

#define IMPORT_CHUNK_SIZE 1000

int SamplePool::ImportSample(const char *name, const char *projectName) {

  if (count_ == MAX_SAMPLES) {
    return -1;
  }

  // Opens file - we assume that have already chdir() into the correct dir
  // that contains the sample file
  auto fs = FileSystem::GetInstance();
  auto fin = fs->Open(name, "r");
  if (!fin) {
    Trace::Error("Failed to open sample input file:%s", name);
    return -1;
  };
  fin->Seek(0, SEEK_END);
  long size = fin->Tell();
  fin->Seek(0, SEEK_SET);

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

  // copy file to current project
  char buffer[IMPORT_CHUNK_SIZE];
  long totalSize = size; // Store original size for progress calculation

  while (size > 0) {
    int count = (size > IMPORT_CHUNK_SIZE) ? IMPORT_CHUNK_SIZE : size;
    fin->Read(buffer, count);
    fout->Write(buffer, 1, count);
    size -= count;

    // Update progress indicator
    int progress = (int)(((totalSize - size) * 100) / totalSize);
    Status::SetMultiLine("Loading:\n%s\n%d%%", projSampleFilename.c_str(),
                         progress);
  };

  // now load the sample into memory/flash from the original source path
  bool status = loadSample(name);
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
  for (int j = i; j < count_ - 1; j++) {
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
