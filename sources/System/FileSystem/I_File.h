/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _I_FILE_H_
#define _I_FILE_H_

struct FileCloser;
class I_File;
bool CloseFile_DO_NOT_USE(I_File *);

class I_File {
public:
  I_File() {}
  virtual ~I_File() {}

  virtual int Read(void *ptr, int size) = 0;
  virtual int GetC() = 0;
  virtual int Write(const void *ptr, int size, int nmemb) = 0;
  virtual void Seek(long offset, int whence) = 0;
  virtual long Tell() = 0;
  virtual int Error() = 0;
  virtual bool Sync() = 0;
  virtual void Dispose() = 0;

protected:
  // Only the filesystem deleter and explicit legacy helpers may close files.
  friend struct FileCloser;
  friend bool CloseFile_DO_NOT_USE(I_File *);
  virtual bool Close() = 0;
};

#endif // _I_FILE_H_
