#include "picoTrackerFileSystem.h"
#include "Adapters/picoTracker/sdcard/sdcard.h"
#include "Application/Utils/wildcard.h"
#include "System/Console/Trace.h"

#include <string>

picoTrackerPagedDir::picoTrackerPagedDir(const char *path) : path_ { std::string(path) }{
  fileIndexes_.reserve(256);
  subdirIndexes_.reserve(96);
  Trace::Log("PAGEDDIR", "NEW:%s", path_.c_str());
};

void picoTrackerPagedDir::GetContent(const char *mask) {
  Trace::Log("PAGEDFILESYSTEM", "GetContent path:%s mask:%s", path_.c_str(), mask);
  fileIndexes_.clear();
  subdirIndexes_.clear();
  FsBaseFile dir;

  if (!dir.open(path_.c_str())) {
    Trace::Error("Failed to open %s", path_.c_str());
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
      Trace::Log("PAGEDFILESYSTEM", "[%d] readdir:%s", fileIndex, current);
    } else if (wildcardfit(mask, current)) {
      fileIndexes_.push_back(fileIndex);
      Trace::Log("PAGEDFILESYSTEM", "[%d] readfile:%s", fileIndex, current);
    }
    count++;
  }
 
  Trace::Log("PAGEDFILESYSTEM", "scanned %d files", count);
}

void picoTrackerPagedDir::getFileList(int startOffset, std::vector<FileListItem> *fileList) {
  Trace::Log("PAGEDFILESYSTEM", "getfile List path:%s", path_.c_str());
  Trace::Log("PAGEDFILESYSTEM", "getfile List dirs:%d files:%d", subdirIndexes_.size(), fileIndexes_.size());
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
  // Max filename is actually 256 per FAT std 
  static const int MAX_FILENAME_LEN = 128;
  char current[MAX_FILENAME_LEN];
  FsBaseFile file;

	if (startOffset == 0 && (path_ != std::string("/samplelib"))) {
		// Insert a parent dir path given that FatFS doesn't provide it
		fileList->push_back(FileListItem("..", 0, true));
	}

  unsigned int count = startOffset;
  for(; count < subdirIndexes_.size() && (fileList->size() < MAX_ITEMS); count++) {
    int index = subdirIndexes_[count];
    Trace::Log("PAGEDFILESYSTEM", "getdir at dir:%d Index %d", dir, index);
    if (!file.open(&dir, index, O_READ)) {
      Trace::Error("PAGEDFILESYSTEM Failed to getfile at Index %d", index);
    }
    int len = file.getName(current, MAX_FILENAME_LEN);
    Trace::Log("PAGEDFILESYSTEM", "dir getName at Index %d length:%d", index, len);
    current[23] = 0; //truncate at 22 char length string
    fileList->push_back(FileListItem(current, index, true));
    Trace::Log("PAGEDFILESYSTEM", "gotdir name:%s", current);
  }
  for(; count < fileIndexes_.size() && (fileList->size() < MAX_ITEMS); count++) {
    int index = fileIndexes_[count];
    Trace::Log("PAGEDFILESYSTEM", "getfile at Index %d", index);
    if (!file.open(&dir, index, O_READ)){
      Trace::Error("PAGEDFILESYSTEM Failed to getfile at Index %d", index);
    }
    int len = file.getName(current, MAX_FILENAME_LEN);
    Trace::Log("PAGEDFILESYSTEM", "file getName at Index %d length:%d", index, len);
    current[25] = 0; //truncate at 24 char length string
    fileList->push_back(FileListItem(current, index, false));
    Trace::Log("PAGEDFILESYSTEM", "gotfilename:%s", current);
  }

  fileCount_ = subdirIndexes_.size() + fileIndexes_.size();
   Trace::Log("PAGEDFILESYSTEM", "fileCount:%d", fileCount_);
}

int picoTrackerPagedDir::size() {
  return fileCount_;
}

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
  return new picoTrackerPagedDir { path };
}

