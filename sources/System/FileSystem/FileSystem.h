#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "Foundation/T_Factory.h"
#include "Foundation/T_SimpleList.h"
#include "Foundation/Types/Types.h"
#include "System/Errors/Result.h"
#include "System/System/System.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#define MAX_FILENAME_SIZE 256

#define SAMPLE_LIB_PATH "/samplelib"

#define PAGED_PAGE_SIZE 18

enum FileType { FT_UNKNOWN, FT_FILE, FT_DIR };
#ifdef PICOBUILD
enum FileSystemStatus { FSOK, FSNotPresent, FSPartitionNotPresent, FSUnknown };
#endif

struct FileListItem {
public:
  // truncate to 22 or 24 chars depending on if dir as they need surronding "[]"
  FileListItem(const char *name, int idx, bool isDir)
      : name{std::string(name)}, index{idx}, isDirectory{isDir} {};

  ~FileListItem() { Trace::Log("FileListItem", "destruct"); };

  std::string name;
  int index;
  bool isDirectory;
};

class Path {
public:
  Path();
  Path(const char *path);
  Path(const std::string &path);
  Path(const Path &path);

  virtual ~Path();

  Path &operator=(const Path &other);

  Path Descend(const std::string &leaf);

  std::string GetPath() const;
  std::string GetCanonicalPath();
  std::string GetName();
  Path GetParent();

  int Compare(const Path &other);

  bool Exists();

  bool IsFile();

  bool IsDirectory();

  bool Matches(const char *pattern);

  static void SetAlias(const char *alias, const char *path);

protected:
  static const char *resolveAlias(const char *alias);
  void getType();

private:
  class Alias {
  public:
    Alias(const char *alias, const char *path);
    void SetPath(const char *);
    const char *GetPath();
    void SetAliasName(const char *);
    const char *GetAliasName();

  private:
    std::string path_;
    std::string alias_;
  };

  static T_SimpleList<Alias> aliases_;

private:
  FileType type_;
  bool gotType_;
  char *path_;
  mutable std::string fullPath_;
};

class I_File {
public:
  virtual ~I_File(){};
  virtual int Read(void *ptr, int size, int nmemb) = 0;
  virtual int GetC() = 0;
  virtual int Write(const void *ptr, int size, int nmemb) = 0;
  virtual void Printf(const char *format, ...) = 0;
  virtual void Seek(long offset, int whence) = 0;
  virtual long Tell() = 0;
  virtual void Close() = 0;
  virtual int Error() = 0;
};

class I_Dir : public T_SimpleList<Path> {
public:
  I_Dir(const char *path) : T_SimpleList<Path>(true) {
    path_ = (char *)SYS_MALLOC((int)strlen(path) + 1);
    strcpy(path_, path);
  };
  virtual ~I_Dir() {
    if (path_)
      free(path_);
  };
  virtual void GetContent(const char *mask) = 0;
  void Compare(Path &p1, Path &p2);

protected:
  char *path_;
};

class I_PagedDir {
public:
  I_PagedDir();
  I_PagedDir(const char *path);
  virtual ~I_PagedDir();
  virtual void GetContent(const char *mask) = 0;
  virtual std::string getFullName(int index) = 0;
  virtual void getFileList(int startIndex,
                           std::vector<FileListItem> *fileList) = 0;
  virtual int size() = 0;

protected:
  unsigned int fileCount_ = 0;
};

class FileSystem : public T_Factory<FileSystem> {
public:
  virtual I_File *Open(const char *path, const char *mode) = 0;
  virtual I_Dir *Open(const char *path) = 0;
#ifdef PICOBUILD
  virtual I_PagedDir *OpenPaged(const char *path) = 0;
  virtual FileSystemStatus GetFSStatus() = 0;
#endif
  virtual Result MakeDir(const char *path) = 0;
  virtual void Delete(const char *) = 0;
  virtual FileType GetFileType(const char *path) = 0;
};

#define FS_FOPEN(a, b) FileSystem::GetInstance()->Open(a, b)

#endif
