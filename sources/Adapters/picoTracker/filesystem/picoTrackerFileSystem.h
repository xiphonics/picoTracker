#ifndef _PICOTRACKERFILESYSTEM_H_
#define _PICOTRACKERFILESYSTEM_H_

#include "Externals/SdFat/src/SdFat.h"
#include "System/FileSystem/FileSystem.h"
#include <stdio.h>
#include <string.h>
#include <vector>

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

class picoTrackerPagedDir : public I_PagedDir {
public:
  picoTrackerPagedDir(const char *path);
  virtual ~picoTrackerPagedDir() {
    Trace::Log("PAGEDDIR", "Destruct:%s", path_.c_str());
  };
  void GetContent(const char *mask);
  std::string getFullName(int index);
  void getFileList(int startIndex, std::vector<FileListItem> *fileList);
  int size();

private:
  const std::string path_;
  std::vector<int> fileIndexes_{};
  std::vector<int> subdirIndexes_{};
};

class picoTrackerFileSystem : public FileSystem {
public:
  picoTrackerFileSystem();
  virtual I_File *Open(const char *path, const char *mode);
  virtual I_Dir *Open(const char *path);
  virtual I_PagedDir *OpenPaged(const char *path);
  virtual FileType GetFileType(const char *path);
  virtual Result MakeDir(const char *path);
  virtual void Delete(const char *){};

private:
  SdFs SD_;
};

#endif
