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

#define MAX_FILE_INDEX_SIZE 256
#define PFILENAME_SIZE 128
#define MAX_PROJECT_SAMPLE_PATH_LENGTH 146 // 17 + 128 + 1

extern FATFS SDFatFS;
extern char SDPath[4]; /* SD logical drive path */

class PI_File : public I_File {
public:
  PI_File(FIL file); // OK
  ~PI_File(){};
  int Read(void *ptr, int size) override; // OK
  int GetC() override;
  int Write(const void *ptr, int size, int nmemb) override;
  void Seek(long offset, int whence) override; // OK
  long Tell() override;
  bool Close() override;
  int Error() override;
  bool Sync() override;

private:
  FIL file_;
};

class advFileSystem : public FileSystem {
public:
  advFileSystem(); // OK
  virtual ~advFileSystem(){};
  virtual I_File *Open(const char *name, const char *mode) override; // OK
  virtual bool chdir(const char *path) override;                     // OK
  virtual void list(etl::ivector<int> *fileIndexes, const char *filter,
                    bool subDirOnly) override;                          // OK
  virtual void getFileName(int index, char *name, int length) override; // OK
  virtual PicoFileType getFileType(int index) override;                 // OK
  virtual bool isParentRoot() override;
  virtual bool isCurrentRoot() override;
  virtual bool DeleteFile(const char *name) override;                  // OK
  virtual bool DeleteDir(const char *name) override;                   // OK
  virtual bool exists(const char *path) override;                      // OK
  virtual bool makeDir(const char *path, bool pFlag = false) override; // OK
  virtual uint64_t getFileSize(int index) override;                    // OK
  virtual bool CopyFile(const char *src, const char *dest) override;   // OK

private:
  //  SdFs sd;
  void tolowercase(char *temp);
  FILINFO fileFromIndex(int index);
  // buffer needs to be allocated here as too big for allocation as local
  // variable on the stack
  uint8_t fileBuffer_[512];
  TCHAR filepath[256];
};

#endif
