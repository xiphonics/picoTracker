#ifndef _PICOTRACKER_FILESYSTEM_H_
#define _PICOTRACKER_FILESYSTEM_H_

#include "Adapters/picoTracker/sdcard/sdcard.h"
#include "Externals/SdFat/src/SdFat.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "System/FileSystem/I_File.h"
#include "pico/sync.h"
#include <mutex>

// Forward declaration
class picoTrackerFile;

// This is the concrete implementation of the FileSystem interface for
// picoTracker
class picoTrackerFileSystem : public FileSystem {
public:
  picoTrackerFileSystem();
  virtual ~picoTrackerFileSystem() {}

  // FileSystem interface implementation
  virtual I_File *Open(const char *name, const char *mode) override;
  virtual bool chdir(const char *path) override;
  virtual void list(etl::ivector<int> *fileIndexes, const char *filter,
                    bool subDirOnly) override;
  virtual void getFileName(int index, char *name, int length) override;
  virtual PicoFileType getFileType(int index) override;
  virtual bool isParentRoot() override;
  virtual bool DeleteFile(const char *name) override;
  virtual bool DeleteDir(const char *name) override;
  virtual bool exists(const char *path) override;
  virtual bool makeDir(const char *path, bool pFlag = false) override;
  virtual uint64_t getFileSize(int index) override;
  virtual bool CopyFile(const char *src, const char *dest) override;

private:
  SdFs sd;
  void tolowercase(char *temp);
  // buffer needs to be allocated here as too big for allocation as local
  // variable on the stack
  uint8_t fileBuffer_[512];
};

// Concrete implementation of PI_File for picoTracker
class picoTrackerFile : public I_File {
public:
  picoTrackerFile(FsBaseFile file);
  virtual ~picoTrackerFile() {}

  // PI_File interface implementation
  virtual int Read(void *ptr, int size) override;
  virtual int GetC() override;
  virtual int Write(const void *ptr, int size, int nmemb) override;
  virtual void Seek(long offset, int whence) override;
  virtual long Tell() override;
  virtual bool Close() override;
  virtual bool DeleteFile() override;
  virtual int Error() override;

private:
  FsBaseFile file_;
};

// Mutex implementation for thread safety
struct Mutex {
  Mutex() { mutex_init(&mutex); }
  void lock() { mutex_enter_blocking(&mutex); }
  void unlock() { mutex_exit(&mutex); }
  Mutex(const Mutex &) = delete;
  Mutex &operator=(const Mutex &) = delete;

private:
  mutex_t mutex;
};

#endif // _PICOTRACKER_FILESYSTEM_H_
