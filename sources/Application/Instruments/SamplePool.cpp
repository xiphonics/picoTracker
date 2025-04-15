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

#ifdef LOAD_IN_FLASH
#include "hardware/flash.h"
// #define FLASH_TARGET_OFFSET (1024 * 1024)
//  Use all flash available after binary for samples
//  WARNING! should be conscious to always ensure 1MB of free space
extern char __flash_binary_end;
#define FLASH_TARGET_OFFSET                                                    \
  ((((uintptr_t) & __flash_binary_end - 0x10000000u) / FLASH_SECTOR_SIZE) +    \
   1) *                                                                        \
      FLASH_SECTOR_SIZE
// #define FLASH_LIMIT (2 * 1024 * 1024)

int SamplePool::flashEraseOffset_ = FLASH_TARGET_OFFSET;
int SamplePool::flashWriteOffset_ = FLASH_TARGET_OFFSET;
int SamplePool::flashLimit_ =
    2 * 1024 * 1024; // default 2mb for the Raspberry Pi Pico

// From the SDK, values are not defined in the header file
#define FLASH_RUID_DUMMY_BYTES 4
#define FLASH_RUID_DATA_BYTES 8
#define FLASH_RUID_TOTAL_BYTES                                                 \
  (1 + FLASH_RUID_DUMMY_BYTES + FLASH_RUID_DATA_BYTES)

#endif

#ifdef PICOBUILD
uint storage_get_flash_capacity() {
  uint8_t txbuf[FLASH_RUID_TOTAL_BYTES] = {0x9f};
  uint8_t rxbuf[FLASH_RUID_TOTAL_BYTES] = {0};
  flash_do_cmd(txbuf, rxbuf, FLASH_RUID_TOTAL_BYTES);

  return 1 << rxbuf[3];
}
#endif

SamplePool::SamplePool() : Observable(&observers_) {
  for (int i = 0; i < MAX_PIG_SAMPLES; i++) {
    names_[i] = NULL;
    wav_[i] = NULL;
  };
  count_ = 0;
#ifdef PICOBUILD
  flashLimit_ = storage_get_flash_capacity();
  Trace::Debug("Flash size is %i bytes", flashLimit_);
#endif
};

SamplePool::~SamplePool() {
  for (int i = 0; i < MAX_PIG_SAMPLES; i++) {
    SAFE_DELETE(wav_[i]);
    SAFE_FREE(names_[i]);
  };
};

void SamplePool::Reset() {
  count_ = 0;
  for (int i = 0; i < MAX_PIG_SAMPLES; i++) {
    SAFE_DELETE(wav_[i]);
    SAFE_FREE(names_[i]);
  };

#ifdef LOAD_IN_FLASH
  // Reset flash erase and write pointers when we close project
  flashEraseOffset_ = FLASH_TARGET_OFFSET;
  flashWriteOffset_ = FLASH_TARGET_OFFSET;
#endif
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
  for (size_t i = 0; i < fileIndexes.size(); i++) {
    fs->getFileName(fileIndexes[i], name, PFILENAME_SIZE);
    if (fs->getFileType(fileIndexes[i]) == PFT_FILE) {
      Status::Set("Loading:%s", name);
      loadSample(name);
    }
    if (i == MAX_PIG_SAMPLES) {
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
    SoundSource *tWav = wav_[index];
    char *tName = names_[index];
    wav_[index] = wav_[rest - 1];
    names_[index] = names_[rest - 1];
    wav_[rest - 1] = tWav;
    names_[rest - 1] = tName;
    rest--;
  };
};

SoundSource *SamplePool::GetSource(int i) { return wav_[i]; };

char **SamplePool::GetNameList() { return names_; };

int SamplePool::GetNameListSize() { return count_; };

bool SamplePool::loadSample(const char *name) {
  Trace::Log("SAMPLEPOOL", "Loading sample into flash: %s", name);

  if (count_ == MAX_PIG_SAMPLES)
    return false;

  WavFile *wave = WavFile::Open(name);
  if (wave) {
    // Get sample information
    int size = wave->GetSize(-1);
    int channels = wave->GetChannelCount(-1);
    int bytePerSample = (wave->GetSampleRate(-1) == 8) ? 1 : 2;
    int totalBytes = size * channels * bytePerSample;

    // Check if this is a single cycle waveform
    bool isSingleCycle = (totalBytes < 1024);

    Trace::Debug("Sample info: size=%d samples, channels=%d, total bytes=%d, "
                 "isSingleCycle=%d",
                 size, channels, totalBytes, isSingleCycle);

    wav_[count_] = wave;
    names_[count_] = (char *)SYS_MALLOC(strlen(name) + 1);
    strcpy(names_[count_], name);
    count_++;
#ifdef LOAD_IN_FLASH
    // For single cycle waveforms, ensure we have enough buffer space
    if (isSingleCycle) {
      Trace::Debug("Loading single cycle waveform into flash: erase "
                   "offset=0x%X, write offset=0x%X",
                   flashEraseOffset_, flashWriteOffset_);
    }
    wave->LoadInFlash(flashEraseOffset_, flashWriteOffset_, flashLimit_);
#else
    wave->GetBuffer(0, wave->GetSize(-1));
#endif
    wave->Close();
    return true;
  } else {
    Trace::Error("Failed to load sample:%s", name);
    return false;
  }
}

#define IMPORT_CHUNK_SIZE 1000

int SamplePool::ImportSample(char *name, const char *projectName) {

  if (count_ == MAX_PIG_SAMPLES) {
    return -1;
  }

  // Opens file - we assume that have already chdir() into the correct dir
  // that contains the sample file
  auto fs = FileSystem::GetInstance();
  I_File *fin = fs->Open(name, "r");
  if (!fin) {
    Trace::Error("Failed to open sample input file:%s", name);
    return -1;
  };
  fin->Seek(0, SEEK_END);
  long size = fin->Tell();
  fin->Seek(0, SEEK_SET);

  etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> projectSamplePath("/projects/");
  projectSamplePath.append(projectName);
  projectSamplePath.append("/samples/");
  projectSamplePath.append(name);
  Status::Set("Loading %s->", name);

  I_File *fout =
      FileSystem::GetInstance()->Open(projectSamplePath.c_str(), "w");
  if (!fout) {
    Trace::Error("Failed to open sample project file:%s", projectSamplePath);
    fin->Close();
    delete (fin);
    return -1;
  };

  // copy file to current project
  char buffer[IMPORT_CHUNK_SIZE];
  while (size > 0) {
    int count = (size > IMPORT_CHUNK_SIZE) ? IMPORT_CHUNK_SIZE : size;
    fin->Read(buffer, count);
    fout->Write(buffer, 1, count);
    size -= count;
  };

  // now load the sample into memory/flash
  bool status = loadSample(name);

  fin->Close();
  fout->Close();
  delete (fin);
  delete (fout);

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
  // delete wav
  SAFE_DELETE(wav_[i]);
  // delete name entry
  SAFE_DELETE(names_[i]);

  // shift all entries from deleted to end
  for (int j = i; j < count_ - 1; j++) {
    wav_[j] = wav_[j + 1];
    names_[j] = names_[j + 1];
  };
  // decrease sample count
  count_--;
  wav_[count_] = 0;
  names_[count_] = 0;

  // now notify observers
  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = i;
  ev.type_ = SPET_DELETE;
  NotifyObservers(&ev);
};
