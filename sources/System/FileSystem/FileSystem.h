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

#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/vector.h"
#include "Foundation/T_Factory.h"
#include "System/FileSystem/FileHandle.h"
#include <cstring>
#include <stdint.h>

#define MAX_FILE_INDEX_SIZE 256
#define PFILENAME_SIZE 256                 // per FAT32 spec for LFNs
#define FAT_MAX_PATH_SIZE 256              // per FAT32 spec for Paths
#define MAX_PROJECT_SAMPLE_PATH_LENGTH 146 // 17 + 128 + 1

enum PicoFileType { PFT_UNKNOWN, PFT_FILE, PFT_DIR };

// Forward declaration
class I_File;

// This is the main FileSystem interface that will be implemented by
// platform-specific classes
class FileSystem : public T_Factory<FileSystem> {
public:
  using PathBuffer = etl::string<FAT_MAX_PATH_SIZE>;

  FileSystem() {}
  virtual ~FileSystem() {}

  virtual FileHandle Open(const char *name, const char *mode) = 0;
  virtual bool chdir(const char *path) = 0;
  virtual bool read(int index, void *data) {
    return false;
  } // Default implementation
  // List files in a full path. Default implementation changes cwd to path.
  virtual bool listPath(etl::ivector<int> *fileIndexes, const char *path,
                        const char *filter, bool subDirOnly) {
    if (!path || !*path) {
      return false;
    }
    if (!chdir(path)) {
      return false;
    }
    list(fileIndexes, filter, subDirOnly);
    return true;
  }
  virtual void list(etl::ivector<int> *fileIndexes, const char *filter,
                    bool subDirOnly) = 0;
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

  // Reusable scratch buffers for building full paths without stack overhead.
  PathBuffer &GetPathBuffer(uint8_t slot = 0) {
    slot = slot < kPathBufferCount ? slot : 0;
    return pathBuffers_[slot];
  }

  // Build a path from up to 5 segments. Returns false if it would overflow.
  bool BuildPath(PathBuffer &out, const char *seg0, const char *seg1 = nullptr,
                 const char *seg2 = nullptr, const char *seg3 = nullptr,
                 const char *seg4 = nullptr, bool absolute = true) {
    out.clear();
    if (absolute) {
      out = "/";
    }

    const char *segments[] = {seg0, seg1, seg2, seg3, seg4};
    for (const char *segment : segments) {
      if (!segment || !*segment) {
        continue;
      }
      const char *s = segment;
      while (*s == '/') {
        ++s;
      }
      if (!*s) {
        continue;
      }
      if (!out.empty() && out.back() != '/') {
        out += '/';
      }
      const size_t segLen = strlen(s);
      if ((out.size() + segLen) > out.capacity()) {
        return false;
      }
      out.append(s, segLen);
    }
    return true;
  }

private:
  static constexpr uint8_t kPathBufferCount = 2;
  PathBuffer pathBuffers_[kPathBufferCount];
};

#endif // _FILESYSTEM_H_
