/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "Externals/etl/include/etl/vector.h"
#include "Foundation/T_Factory.h"
#include "System/FileSystem/FileHandle.h"
#include <stdint.h>

#define MAX_FILE_INDEX_SIZE 256
#define PFILENAME_SIZE 256                 // per FAT32 spec for LFNs
#define MAX_PROJECT_SAMPLE_PATH_LENGTH 146 // 17 + 128 + 1

enum PicoFileType { PFT_UNKNOWN, PFT_FILE, PFT_DIR };

// Forward declaration
class I_File;

// This is the main FileSystem interface that will be implemented by
// platform-specific classes
class FileSystem : public T_Factory<FileSystem> {
public:
  FileSystem() {}
  virtual ~FileSystem() {}

  virtual FileHandle Open(const char *name, const char *mode) = 0;
  virtual bool chdir(const char *path) = 0;
  virtual bool read(int index, void *data) {
    return false;
  } // Default implementation
  virtual void list(etl::ivector<int> *fileIndexes, const char *filter, bool subDirOnly, bool sorted=false) = 0;
  virtual void getFileName(int index, char *name, int length) = 0;
  virtual PicoFileType getFileType(int index) = 0;
  virtual bool isParentRoot() = 0;
  virtual bool isCurrentRoot() = 0;
  virtual bool DeleteFile(const char *name) = 0;
  virtual bool DeleteDir(const char *name) = 0;
  // Optional batching hook for filesystem implementations that cache listings.
  virtual void BeginBatch() {}
  virtual void EndBatch() {}
  virtual bool exists(const char *path) = 0;
  virtual bool makeDir(const char *path, bool pFlag = false) = 0;
  virtual uint64_t getFileSize(int index) = 0;
  virtual bool CopyFile(const char *src, const char *dest) = 0;
  virtual bool isExFat() = 0;
};

#endif // _FILESYSTEM_H_
