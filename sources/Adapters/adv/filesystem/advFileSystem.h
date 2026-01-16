/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _PICO_FILESYSTEM_H_
#define _PICO_FILESYSTEM_H_

#include "Externals/FatFs/source/ff.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/vector.h"
#include "Foundation/T_Factory.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "System/FileSystem/I_File.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

#define MAX_PROJECT_SAMPLE_PATH_LENGTH 146 // 17 + 128 + 1

extern FATFS SDFatFS;
extern char SDPath[4]; /* SD logical drive path */

class PI_File : public I_File {
public:
  PI_File(FIL file);
  ~PI_File();
  int Read(void *ptr, int size) override;
  int GetC() override;
  int Write(const void *ptr, int size, int nmemb) override;
  void Seek(long offset, int whence) override;
  long Tell() override;
  bool Close() override;
  int Error() override;
  bool Sync() override;
  void Dispose() override;

private:
  FIL file_;
  // isOpen_ needed as a safeguard because we still use the legacy explicit
  // close from TinyXML
  bool isOpen_;
};

class advFileSystem : public FileSystem {
public:
  advFileSystem(); // OK
  virtual ~advFileSystem(){};
  virtual FileHandle Open(const char *name, const char *mode) override;
  virtual bool chdir(const char *path) override;
  virtual void list(etl::ivector<int> *fileIndexes, const char *filter,
                    bool subDirOnly) override;
  virtual void getFileName(int index, char *name, int length) override;
  virtual PicoFileType getFileType(int index) override;
  virtual bool isParentRoot() override;
  virtual bool isCurrentRoot() override;
  virtual bool DeleteFile(const char *name) override;
  virtual bool DeleteDir(const char *name) override;
  virtual bool exists(const char *path) override;
  virtual bool makeDir(const char *path, bool pFlag = false) override;
  virtual uint64_t getFileSize(int index) override;
  virtual bool CopyFile(const char *src, const char *dest) override;

private:
  FILINFO fileFromIndex(int index);
  void tolowercase(char *temp);
  void updateCache();
  TCHAR filepath[PFILENAME_SIZE];
  BYTE fileBuffer_[512];

  // TODO: this is quite big! need move it out to external RAM
  // cache for fileFromIndex
  etl::vector<FILINFO, MAX_FILE_INDEX_SIZE> file_cache_;
};

#endif
