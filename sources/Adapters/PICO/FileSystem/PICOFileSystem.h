#ifndef _PICO_FILESYSTEM_H_
#define _PICO_FILESYSTEM_H_

#include "Externals/SdFat/src/SdFat.h"
#include "System/FileSystem/FileSystem.h"
#include <stdio.h>
#include <string.h>

class PICOFile : public I_File {
public:
  PICOFile(FsBaseFile file);
  virtual int Read(void *ptr, int size, int nmemb);
  virtual int GetC();
  virtual int Write(const void *ptr, int size, int nmemb);
  virtual void Printf(const char *format, ...);
  virtual void Seek(long offset, int whence);
  virtual long Tell();
  virtual void Close();
  virtual int Error();

private:
  FsBaseFile file_;
};

class PICODir : public I_Dir {
public:
  PICODir(const char *path);
  virtual ~PICODir(){};
  virtual void GetContent(char *mask);
};

class PICOFileSystem : public FileSystem {
public:
  PICOFileSystem();
  virtual I_File *Open(const char *path, char *mode);
  virtual I_Dir *Open(const char *path);
  virtual FileType GetFileType(const char *path);
  virtual Result MakeDir(const char *path);
  virtual void Delete(const char *){};

private:
  SdFs SD_;
};
#endif
