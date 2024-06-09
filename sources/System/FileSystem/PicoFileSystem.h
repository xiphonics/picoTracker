#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "Externals/etl/include/etl/array.h"
#include "Foundation/T_Factory.h"

enum FileType { FT_UNKNOWN, FT_FILE, FT_DIR };

class PicoFileSystem : public T_Factory<PicoFileSystem> {
public:
  bool chdir(const char *path);
  bool read(int index, void *data);
  void list(etl::array<int, 256> *fileIndexes);
  void getFileName(int index, char *name, int length);
  FileType getFileType(int index);
};

#endif