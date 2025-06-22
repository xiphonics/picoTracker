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

class I_File {
public:
  I_File() {}
  virtual ~I_File() {}

  virtual int Read(void *ptr, int size) = 0;
  virtual int GetC() = 0;
  virtual int Write(const void *ptr, int size, int nmemb) = 0;
  virtual void Seek(long offset, int whence) = 0;
  virtual long Tell() = 0;
  virtual bool Close() = 0;
  virtual bool DeleteFile() = 0;
  virtual int Error() = 0;
};

#endif // _I_FILE_H_
