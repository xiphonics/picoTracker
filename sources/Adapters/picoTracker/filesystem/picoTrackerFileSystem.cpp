#include "picoTrackerFileSystem.h"
#include "Adapters/picoTracker/sdcard/sdcard.h"
#include "Application/Utils/wildcard.h"
#include "System/Console/Trace.h"

#include <string>

// Max filename is actually 256 per FAT std
static const int MAX_FILENAME_LEN = 128;

picoTrackerPagedDir::picoTrackerPagedDir(const char *path)
    : path_{std::string(path)} {
  // Initial size allocation for file and subdir entries per each dir
  // these act as "soft limits" since while the Vector objects will allocate
  // more room if the initially allocated space is used up, this will likely 
  // exhaust available RAM per my testing as of making this change
  fileIndexes_.reserve(354);
  subdirIndexes_.reserve(96);
};

void picoTrackerPagedDir::GetContent(const char *mask) {
  Trace::Log("PAGEDFILESYSTEM", "GetContent path:%s mask:%s", path_.c_str(),
             mask);
  fileIndexes_.clear();
  subdirIndexes_.clear();
  FsBaseFile dir;

  if (!dir.open(path_.c_str())) {
    Trace::Error("PagedDir GetContent Failed to open %s", path_.c_str());
    return;
  }

  if (!dir.isDir()) {
    Trace::Error("Path:%s is not a directory", path_.c_str());
    return;
  }

  int count = 0;
  FsBaseFile entry;
  while (entry.openNext(&dir, O_READ)) {
    char current[MAX_FILENAME_SIZE];
    entry.getName(current, MAX_FILENAME_SIZE);

    int fileIndex = entry.dirIndex();
    if (entry.isDir()) {
      subdirIndexes_.push_back(fileIndex);
    } else if (wildcardfit(mask, current)) {
      fileIndexes_.push_back(fileIndex);
    }
    count++;
  }
  Trace::Log("PAGEDFILESYSTEM", "scanned %d files", count);
}

std::string picoTrackerPagedDir::getFullName(int index) {
  FsBaseFile dir;
  if (!dir.open(path_.c_str())) {
    Trace::Error("getFileList failed open:%s", path_.c_str());
    return std::string("");
  }
  if (!dir.isDir()) {
    Trace::Error("Path:%s is not a directory", path_.c_str());
    return std::string("");
  }
  char filename[MAX_FILENAME_LEN];
  FsBaseFile file;

  if (!file.open(&dir, index, O_READ)) {
    Trace::Error("PAGEDFILESYSTEM Failed to getfile at Index %d", index);
  }
  file.getName(filename, MAX_FILENAME_LEN);
  return std::string(filename);
}

void picoTrackerPagedDir::getFileList(int startOffset,
                                      std::vector<FileListItem> *fileList) {
  bool addedParentDirEntry = false;
  FsBaseFile dir;

  if (!dir.open(path_.c_str())) {
    Trace::Error("getFileList failed open:%s", path_.c_str());
    return;
  }
  if (!dir.isDir()) {
    Trace::Error("Path:%s is not a directory", path_.c_str());
    return;
  }
  static const int MAX_ITEMS = 15;
  char current[MAX_FILENAME_LEN];
  FsBaseFile file;

  if (startOffset == 0 && (path_ != std::string(SAMPLE_LIB_PATH))) {
    // Insert a parent dir path given that FatFS doesn't provide it
    fileList->push_back(FileListItem("..", 0, true));
    addedParentDirEntry = true;
  }

  unsigned int count = startOffset;
  for (; count < subdirIndexes_.size() && (fileList->size() < MAX_ITEMS);
       count++) {
    int index = subdirIndexes_[count];
    if (!file.open(&dir, index, O_READ)) {
      Trace::Error("PAGEDFILESYSTEM Failed to getfile at Index %d", index);
    }
    file.getName(current, MAX_FILENAME_LEN);
    current[23] = 0; // truncate at 22 char length string
    fileList->push_back(FileListItem(current, index, true));
  }
  for (; count < fileIndexes_.size() && (fileList->size() < MAX_ITEMS);
       count++) {
    int index = fileIndexes_[count];
    if (!file.open(&dir, index, O_READ)) {
      Trace::Error("PAGEDFILESYSTEM Failed to getfile at Index %d", index);
    }
    file.getName(current, MAX_FILENAME_LEN);
    current[25] = 0; // truncate at 24 char length string
    fileList->push_back(FileListItem(current, index, false));
  }

  // +1 is for the synthezied Parent dir entry of ".."
  fileCount_ = subdirIndexes_.size() + fileIndexes_.size() +
               (addedParentDirEntry ? 1 : 0);
}

int picoTrackerPagedDir::size() { return fileCount_; }

picoTrackerDir::picoTrackerDir(const char *path) : I_Dir(path){};

void picoTrackerDir::GetContent(const char *mask) {
  Trace::Log("FILESYSTEM", "GetContent %s with mask %s", path_, mask);
  Empty();
  FsBaseFile dir;

  if (!dir.open(path_)) {
    Trace::Error("Failed to open %s", path_);
    return;
  }

  if (!dir.isDir()) {
    Trace::Error("Path \"%s\" is not a directory", path_);
    return;
  }

  int count = 0;

  FsBaseFile entry;
  while (entry.openNext(&dir, O_READ)) {
    char current[MAX_FILENAME_SIZE];
    entry.getName(current, MAX_FILENAME_SIZE);
    char *c = current;
    while (*c) {
      *c = tolower(*c);
      c++;
    }

    if (wildcardfit(mask, current)) {
      entry.getName(current, MAX_FILENAME_SIZE);
      std::string fullpath = path_;
      if (path_[strlen(path_) - 1] != '/') {
        fullpath += "/";
      }
      fullpath += current;
      Path *path = new Path(fullpath.c_str());
      Insert(path);
    }
    count++;
  }
  // Insert a parent dir path given that FatFS doesn't provide it
  Path cur(this->path_);
  Path *parent = new Path(cur.GetParent().GetPath());
  Insert(parent);
  dir.close();
}

picoTrackerFile::picoTrackerFile(FsBaseFile file) { file_ = file; };

int picoTrackerFile::Read(void *ptr, int size, int nmemb) {
  return file_.read(ptr, size * nmemb);
}

int picoTrackerFile::GetC() { return file_.read(); }

int picoTrackerFile::Write(const void *ptr, int size, int nmemb) {
  return file_.write(ptr, size * nmemb);
}

void picoTrackerFile::Printf(const char *fmt, ...) {
  Trace::Debug("picoTrackerFileSystem::Printf called...");
  // TODO: What is this for?
}

void picoTrackerFile::Seek(long offset, int whence) {
  switch (whence) {
  case SEEK_SET:
    file_.seek(offset);
    break;
  case SEEK_CUR:
    file_.seekCur(offset);
    break;
  case SEEK_END:
    file_.seekEnd(offset);
    break;
  default:
    Trace::Error("Invalid seek whence: %s", whence);
  }
}

long picoTrackerFile::Tell() { return file_.curPosition(); }

void picoTrackerFile::Close() { file_.close(); }

int picoTrackerFile::Error() { return file_.getError(); }

picoTrackerFileSystem::picoTrackerFileSystem() {

  // Check for the common case, FAT filesystem as first partition
  Trace::Log("FILESYSTEM", "Try to mount SD Card");
  if (SD_.begin(SD_CONFIG)) {
    Trace::Log("FILESYSTEM",
               "Mounted SD Card FAT Filesystem from first partition");
    return;
  }

  // Do we have any kind of card?
  if (!SD_.card() || SD_.sdErrorCode() != 0) {
    Trace::Log("FILESYSTEM", "No SD Card present");
    return;
  }
  // Try to mount the whole card as FAT (without partition table)
  if (static_cast<FsVolume *>(&SD_)->begin(SD_.card(), true, 0)) {
    Trace::Log("FILESYSTEM",
               "Mounted SD Card FAT Filesystem without partition table");
    return;
  }
}

I_File *picoTrackerFileSystem::Open(const char *path, const char *mode) {
  Trace::Log("FILESYSTEM", "Open file %s, mode: %s", path, mode);
  oflag_t rmode;
  switch (*mode) {
  case 'r':
    rmode = O_RDONLY;
    break;
  case 'w':
    rmode = O_WRONLY | O_CREAT | O_TRUNC;
    break;
  default:
    Trace::Error("Invalid mode: %s", mode);
    return 0;
  }

  FsBaseFile file;
  picoTrackerFile *wFile = 0;
  if (file.open(path, rmode)) {
    wFile = new picoTrackerFile(file);
  }
  return wFile;
};

I_Dir *picoTrackerFileSystem::Open(const char *path) {
  Trace::Log("FILESYSTEM", "Open dir %s", path);
  return new picoTrackerDir(path);
};

Result picoTrackerFileSystem::MakeDir(const char *path) {
  Trace::Log("FILESYSTEM", "Make dir %s", path);
  if (!SD_.mkdir(path, false)) {
    std::string result = "Could not create path ";
    result += path;
    return Result(result);
  }
  return Result::NoError;
};

FileType picoTrackerFileSystem::GetFileType(const char *path) {
  Trace::Log("FILESYSTEM", "Get File type %s", path);
  FileType filetype;
  FsBaseFile file;
  if (!file.open(path, O_READ)) {
    return FT_UNKNOWN;
  }
  if (file.isDirectory()) {
    Trace::Debug("%s is directory", path);
    filetype = FT_DIR;
  } else if (file.isFile()) {
    Trace::Debug("%s is regular file", path);
    filetype = FT_FILE;
  } else {
    filetype = FT_UNKNOWN;
  }

  file.close();
  return filetype;
};

I_PagedDir *picoTrackerFileSystem::OpenPaged(const char *path) {
  return new picoTrackerPagedDir{path};
}
