/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _FILEHANDLE_H_
#define _FILEHANDLE_H_

#include "System/FileSystem/I_File.h"
#include <memory>

// Custom deleter that closes the file before releasing the allocation.
struct FileCloser {
  void operator()(I_File *file) const {
    if (file) {
      file->Dispose();
    }
  }
};

// A unique, non-copyable handle for files that does not expose `release()`.
class FileHandle {
public:
  FileHandle() = default;
  explicit FileHandle(I_File *file) : ptr_(file) {}

  FileHandle(FileHandle &&) noexcept = default;
  FileHandle &operator=(FileHandle &&) noexcept = default;

  FileHandle(const FileHandle &) = delete;
  FileHandle &operator=(const FileHandle &) = delete;

  I_File *get() const { return ptr_.get(); }
  I_File &operator*() const { return *ptr_; }
  I_File *operator->() const { return ptr_.get(); }
  explicit operator bool() const { return static_cast<bool>(ptr_); }

  void reset(I_File *file = nullptr) { ptr_.reset(file); }

private:
  std::unique_ptr<I_File, FileCloser> ptr_;

  // For legacy interop (e.g., adapters that require a raw pointer),
  // use AcquireLegacyFileHandle to explicitly leak ownership.
  friend I_File *AcquireLegacyFileHandle_DO_NOT_USE(FileHandle &&handle);
  friend bool CloseFile_DO_NOT_USE(I_File *file);
};

inline FileHandle MakeFileHandle(I_File *file) { return FileHandle(file); }

inline I_File *AcquireLegacyFileHandle_DO_NOT_USE(FileHandle &&handle) {
  return handle.ptr_.release();
}

inline bool CloseFile_DO_NOT_USE(I_File *file) {
  if (!file) {
    return true;
  }
  bool ok = file->Close();
  file->Dispose();
  return ok;
}

#endif // _FILEHANDLE_H_
