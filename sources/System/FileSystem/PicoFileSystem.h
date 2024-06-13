#ifndef _PICO_FILESYSTEM_H_
#define _PICO_FILESYSTEM_H_

#include "Adapters/picoTracker/sdcard/sdcard.h"
#include "Application/Utils/wildcard.h"
#include "Externals/SdFat/src/SdFat.h"
#include "Externals/etl/include/etl/array.h"
#include "Foundation/T_Factory.h"
#include "System/Console/Trace.h"

enum PicoFileType { PFT_UNKNOWN, PFT_FILE, PFT_DIR };

class PicoFileSystem : public T_Singleton<PicoFileSystem> {
public:
  PicoFileSystem();
  bool chRootDir();
  bool chdir(const char *path);
  bool read(int index, void *data);
  void list(etl::array<int, 256> *fileIndexes);
  void getFileName(int index, char *name, int length);
  PicoFileType getFileType(int index);

private:
  SdFs sd;
  FsFile cwd;
};

#endif