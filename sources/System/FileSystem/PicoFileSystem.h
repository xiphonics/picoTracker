#ifndef _PICO_FILESYSTEM_H_
#define _PICO_FILESYSTEM_H_

#include "Adapters/picoTracker/sdcard/sdcard.h"
#include "Application/Utils/wildcard.h"
#include "Externals/SdFat/src/SdFat.h"
#include "Externals/etl/include/etl/vector.h"
#include "Foundation/T_Factory.h"
#include "System/Console/Trace.h"

#define MAX_FILE_INDEX_SIZE 256
#define PFILENAME_SIZE 128

enum PicoFileType { PFT_UNKNOWN, PFT_FILE, PFT_DIR };

class PI_File {
public:
  PI_File(FsBaseFile file);
  ~PI_File(){};
  int Read(void *ptr, int size, int nmemb);
  int GetC();
  int Write(const void *ptr, int size, int nmemb);
  void Seek(long offset, int whence);
  long Tell();
  void Close();
  void DeleteFile();
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
  void list(etl::vector<int, MAX_FILE_INDEX_SIZE> *fileIndexes,
            const char *filter, bool subDirOnly);
  void getFileName(int index, char *name, int length);
  PicoFileType getFileType(int index);
  bool isParentRoot();
  void DeleteFile(const char *name);
  bool exists(const char *path);
  bool makeDir(const char *path);
  uint64_t getFileSize(int index);

private:
  SdFs sd;
  void tolowercase(char *temp);
};

#endif