#include "SamplePool.h"
#include "Adapters/picoTracker/utils/stringutils.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "Externals/etl/include/etl/string.h"
#include "System/Console/Trace.h"
#include "System/io/Status.h"
#include <stdlib.h>
#include <string.h>
#include <string>

#define SAMPLE_LIB "root:samplelib"

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

int SamplePool::flashUsage() { return flashLimit_ - flashWriteOffset_; }

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

const char *SamplePool::GetSampleLib() {
  Config *config = Config::GetInstance();
  const char *lib = config->GetValue("SAMPLELIB");
  return lib ? lib : SAMPLE_LIB;
}

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

  Path sampleDir("samples:");

  I_Dir *dir = FileSystem::GetInstance()->Open(sampleDir.GetPath().c_str());
  if (!dir) {
    return;
  }

  // First, find all wav files

  dir->GetContent("*.wav");
  count_ = 0;
  for (dir->Begin(); !dir->IsDone(); dir->Next()) {
    Path &path = dir->CurrentItem();
    Status::Set("Loading %s", path.GetName().c_str());
    loadSample(path.GetPath().c_str());
    if (count_ == MAX_PIG_SAMPLES) {
      Trace::Error("Warning maximum sample count reached");
      break;
    };
  };

  delete dir;

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

int SamplePool::ImportSample(Path &path) {

  if (count_ == MAX_PIG_SAMPLES)
    return -1;

  // construct target path

  std::string dpath = "samples:";
  dpath += path.GetName();
  Path dstPath(dpath.c_str());

  // truncate long file names
  auto fileNameDisplay = etl::string<80>();
  fileNameDisplay.append(path.GetName().length() > 12
                             ? path.GetName().substr(0, 12).c_str()
                             : path.GetName().c_str());

  Status::Set("Loading %s", fileNameDisplay);

  // Opens files

  I_File *fin = FileSystem::GetInstance()->Open(path.GetPath().c_str(), "r");
  if (!fin) {
    Trace::Error("Failed to open input file %s", path.GetPath().c_str());
    return -1;
  };
  fin->Seek(0, SEEK_END);
  long size = fin->Tell();
  fin->Seek(0, SEEK_SET);

  I_File *fout =
      FileSystem::GetInstance()->Open(dstPath.GetPath().c_str(), "w");
  if (!fout) {
    fin->Close();
    delete (fin);
    return -1;
  };

  // copy file to current project
  char buffer[IMPORT_CHUNK_SIZE];
  int fileSize = size;
  char progressbuffer[10];
  while (size > 0) {
    int count = (size > IMPORT_CHUNK_SIZE) ? IMPORT_CHUNK_SIZE : size;
    fin->Read(buffer, 1, count);
    fout->Write(buffer, 1, count);
    size -= count;
    printProgress(((fileSize - size) / (float)fileSize), progressbuffer, false);
    Status::Set("Copying:%s %s", fileNameDisplay.data(), progressbuffer);
  };

  fin->Close();
  fout->Close();
  delete (fin);
  delete (fout);

  Status::Set("Loading to Flash:%s", fileNameDisplay.data());
  // now load the sample
  bool status = loadSample(dstPath.GetPath().c_str());

  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = count_ - 1;
  ev.type_ = SPET_INSERT;
  NotifyObservers(&ev);
  return status ? (count_ - 1) : -1;
};

void SamplePool::PurgeSample(int i) {

  // construct the path of the sample to delete

  std::string wavPath = "samples:";
  wavPath += names_[i];
  Path path(wavPath.c_str());
  // delete wav
  SAFE_DELETE(wav_[i]);
  // delete name entry
  SAFE_DELETE(names_[i]);

  // delete file
  FileSystem::GetInstance()->Delete(path.GetPath().c_str());

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
