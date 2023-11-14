#ifndef _PICOTRACKERFILESYSTEM_H_
#define _PICOTRACKERFILESYSTEM_H_

#include "Externals/SdFat/src/SdFat.h"
#include "System/FileSystem/FileSystem.h"
#include <stdio.h>
#include <string.h>
#include <vector.h>

class picoTrackerFile : public I_File {
public:
  picoTrackerFile(FsBaseFile file);
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

class picoTrackerDir : public I_Dir {
public:
  picoTrackerDir(const char *path);
  virtual ~picoTrackerDir(){};
  virtual void GetContent(const char *mask);
};

class picoTrackerFileSystem : public FileSystem {
public:
  picoTrackerFileSystem();
  virtual I_File *Open(const char *path, const char *mode);
  virtual I_Dir *Open(const char *path);
  virtual FileType GetFileType(const char *path);
  virtual Result MakeDir(const char *path);
  virtual void Delete(const char *){};

private:
  SdFs SD_;
};

// This holds a list of int "indexes" that match strings, stored in alphabetically
// sorted order.
struct SortedIndexList {
public:
  SortedIndexList();

  void Add(const char *name, index);

private:
  std:vector<int> indexes_;
  std:string lastInserted

}

#endif
