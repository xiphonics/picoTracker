#include "SamplePool.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/PicoFileSystem.h"
#include "System/io/Status.h"
#include <stdlib.h>
#include <string.h>
#include <string>

#ifdef LOAD_IN_FLASH
#include "hardware/flash.h"
// #define FLASH_TARGET_OFFSET (1024 * 1024)
//  Use all flash available after binary for samples
//  WARNING! should be conscious to always ensure 1MB of free space
extern char __flash_binary_end;
#define FLASH_TARGET_OFFSET                                                    \
  ((((uintptr_t)&__flash_binary_end - 0x10000000u) / FLASH_SECTOR_SIZE) + 1) * \
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

SamplePool::SamplePool() {
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

void SamplePool::Load() {
  auto picoFS = PicoFileSystem::GetInstance();
  picoFS->chdir("/projects");
  picoFS->chdir(projectName_);
  picoFS->chdir(PROJECT_SAMPLES_DIR);

  // First, find all wav files
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexes;
  picoFS->list(&fileIndexes, ".wav");
  char name[PFILENAME_SIZE];
  for (size_t i = 0; i < fileIndexes.size(); i++) {
    picoFS->getFileName(fileIndexes[i], name, PFILENAME_SIZE);
    if (picoFS->getFileType(fileIndexes[i]) == PFT_FILE) {
      Status::Set("Loading:%s\n", name);
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

bool SamplePool::loadSample(const char *path) {

  if (count_ == MAX_PIG_SAMPLES)
    return false;

  Path wavPath(path);
  WavFile *wave = WavFile::Open(path);
  if (wave) {
    wav_[count_] = wave;
    const std::string name = wavPath.GetName();
    names_[count_] = (char *)SYS_MALLOC(name.length() + 1);
    strcpy(names_[count_], name.c_str());
    count_++;
#ifdef LOAD_IN_FLASH
    wave->LoadInFlash(flashEraseOffset_, flashWriteOffset_, flashLimit_);
#else
    wave->GetBuffer(0, wave->GetSize(-1));
#endif
    wave->Close();
    return true;
  } else {
    Trace::Error("Failed to load samples %s", wavPath.GetName().c_str());
    return false;
  }
}

#define IMPORT_CHUNK_SIZE 1000

int SamplePool::ImportSample(char *name) {

  if (count_ == MAX_PIG_SAMPLES) {
    return -1;
  }

  // Opens file - we assume that have already chdir() into the correct dir
  // that contains the sample file
  auto picoFS = PicoFileSystem::GetInstance();
  PI_File *fin = picoFS->Open(name, "r");
  if (!fin) {
    Trace::Error("Failed to open sample input file:%s\n", name);
    return -1;
  };
  fin->Seek(0, SEEK_END);
  long size = fin->Tell();
  fin->Seek(0, SEEK_SET);

  // TODO: need to truncate sample file names to something like 64chars
  //  so that we dont overflow this temp string
  etl::string<128> projectSamplePath("/projects/");
  projectSamplePath.append(projectName_);
  projectSamplePath.append("/samples/");
  projectSamplePath.append(name);
  Status::Set("Loading %s->", name);

  PI_File *fout =
      PicoFileSystem::GetInstance()->Open(projectSamplePath.c_str(), "w");
  if (!fout) {
    Trace::Error("Failed to open sample project file:%s\n", projectSamplePath);
    fin->Close();
    delete (fin);
    return -1;
  };

  // copy file to current project
  char buffer[IMPORT_CHUNK_SIZE];
  while (size > 0) {
    int count = (size > IMPORT_CHUNK_SIZE) ? IMPORT_CHUNK_SIZE : size;
    fin->Read(buffer, 1, count);
    fout->Write(buffer, 1, count);
    size -= count;
  };

  fin->Close();
  fout->Close();
  delete (fin);
  delete (fout);

  // now load the sample
  bool status = loadSample(name);

  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = count_ - 1;
  ev.type_ = SPET_INSERT;
  NotifyObservers(&ev);
  return status ? (count_ - 1) : -1;
};

void SamplePool::PurgeSample(int i) {
  auto picoFS = PicoFileSystem::GetInstance();

  // TODO use define constants for these strings
  etl::string<256> delPath("/projects/");
  delPath.append(projectName_);
  delPath.append("/samples/");
  delPath.append(names_[i]);

  // delete file
  PicoFileSystem::GetInstance()->DeleteFile(delPath.c_str());
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
