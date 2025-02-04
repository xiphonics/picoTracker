#ifndef _PICO_FILESYSTEM_H_
#define _PICO_FILESYSTEM_H_

#include "Adapters/picoTracker/sdcard/sdcard.h"
#include "Externals/SdFat/src/SdFat.h"
#include "Externals/etl/include/etl/vector.h"
#include "Foundation/T_Factory.h"
#include "System/Console/Trace.h"
#include "pico/sync.h"
#include <mutex>

#define MAX_FILE_INDEX_SIZE 256
#define PFILENAME_SIZE 128
#define MAX_PROJECT_SAMPLE_PATH_LENGTH 146 // 17 + 128 + 1

enum PicoFileType { PFT_UNKNOWN, PFT_FILE, PFT_DIR };

class PI_File {
public:
  PI_File(FsBaseFile file);
  ~PI_File(){};
  int Read(void *ptr, int size);
  int GetC();
  int Write(const void *ptr, int size, int nmemb);
  void Seek(long offset, int whence);
  long Tell();
  bool Close();
  bool DeleteFile();
  int Error();

private:
  FsBaseFile file_;
};

class PicoFileSystem : public T_Singleton<PicoFileSystem> {
public:
  PicoFileSystem();
  PI_File *Open(const char *name, const char *mode);
  bool chdir(const char *path);
  bool read(int index, void *data);
  void list(etl::ivector<int> *fileIndexes, const char *filter,
            bool subDirOnly);
  void getFileName(int index, char *name, int length);
  PicoFileType getFileType(int index);
  bool isParentRoot();
  bool DeleteFile(const char *name);
  bool DeleteDir(const char *name);
  bool exists(const char *path);
  bool makeDir(const char *path);
  uint64_t getFileSize(int index);
  bool CopyFile(const char *src, const char *dest);

private:
  SdFs sd;
  void tolowercase(char *temp);
  // buffer needs to be allocated here as too big for allocation as local
  // variable on the stack
  uint8_t fileBuffer_[512];
};

struct Mutex {
  Mutex() { mutex_init(&mutex); }
  void lock() { mutex_enter_blocking(&mutex); }
  void unlock() { mutex_exit(&mutex); }
  Mutex(const Mutex &) = delete;
  Mutex &operator=(const Mutex &) = delete;

private:
  mutex_t mutex;
};

#endif
