/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "picoTrackerFileSystem.h"

#include "pico/multicore.h"

// Global mutex for thread safety
Mutex mutex;

picoTrackerFileSystem::picoTrackerFileSystem() {
  // init out access mutex
  std::lock_guard<Mutex> lock(mutex);

  // Check for the common case, FAT filesystem as first partition
  Trace::Log("FILESYSTEM", "Try to mount SD Card");
  if (sd.begin(SD_CONFIG)) {
    Trace::Log("FILESYSTEM", "Mounted SD Card FAT Filesystem first partition");
    return;
  }
  // Do we have any kind of card?
  if (!sd.card() || sd.sdErrorCode() != 0) {
    Trace::Log("FILESYSTEM", "No SD Card present");
    return;
  }
  // Try to mount the whole card as FAT (without partition table)
  if (static_cast<FsVolume *>(&sd)->begin(sd.card(), true, 0)) {
    Trace::Log("FILESYSTEM",
               "Mounted SD Card FAT Filesystem without partition table");
    return;
  }
}

I_File *picoTrackerFileSystem::Open(const char *name, const char *mode) {
  Trace::Log("FILESYSTEM", "Open file:%s, mode:%s", name, mode);
  std::lock_guard<Mutex> lock(mutex);
  oflag_t rmode;
  switch (*mode) {
  case 'r':
    rmode = O_RDONLY;
    break;
  case 'w':
    rmode = O_WRONLY | O_CREAT | O_TRUNC;
    break;
  default:
    rmode = O_RDONLY;
    Trace::Error("Invalid mode: %s [%d]", mode, rmode);
    return 0;
  }
  FsBaseFile cwd;
  if (!cwd.openCwd()) {
    return nullptr;
  }
  I_File *wFile = 0;
  if (cwd.open(name, rmode)) {
    wFile = new picoTrackerFile(cwd);
  } else {
    Trace::Error("FILESYSTEM: Cannot open file:%s", name, mode);
  }
  return wFile;
}

bool picoTrackerFileSystem::chdir(const char *name) {
  Trace::Log("FILESYSTEM", "chdir:%s", name);
  std::lock_guard<Mutex> lock(mutex);

  sd.chvol();
  auto res = sd.vol()->chdir(name);
  File cwd;
  char buf[PFILENAME_SIZE];
  cwd.openCwd();
  cwd.getName(buf, 128);
  Trace::Log("FILESYSTEM", "new CWD:%s", buf);
  cwd.close();
  return res;
}

PicoFileType picoTrackerFileSystem::getFileType(int index) {
  std::lock_guard<Mutex> lock(mutex);

  FsBaseFile cwd;
  if (!cwd.openCwd()) {
    char name[PFILENAME_SIZE];
    cwd.getName(name, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd: %s", name);
    return PFT_UNKNOWN;
  }
  FsBaseFile entry;
  entry.open(index);
  auto isDir = entry.isDirectory();
  entry.close();

  return isDir ? PFT_DIR : PFT_FILE;
}

void picoTrackerFileSystem::list(etl::ivector<int> *fileIndexes,
                                 const char *filter, bool subDirOnly) {
  std::lock_guard<Mutex> lock(mutex);

  fileIndexes->clear();

  File cwd;
  if (!cwd.openCwd()) {
    char name[PFILENAME_SIZE];
    cwd.getName(name, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd");
    return;
  }
  char buffer[PFILENAME_SIZE];
  cwd.getName(buffer, PFILENAME_SIZE);
  Trace::Log("FILESYSTEM", "LIST DIR:%s", buffer);

  if (!cwd.isDir()) {
    Trace::Error("Path is not a directory");
    return;
  }

  File entry;
  uint16_t count = 0;

  // ref: https://github.com/greiman/SdFat/issues/353#issuecomment-1003422848
  while (entry.openNext(&cwd, O_READ) && (count < fileIndexes->capacity())) {
    uint32_t index = entry.dirIndex();
    entry.getName(buffer, PFILENAME_SIZE);

    bool matchesFilter = true;
    if (strlen(filter) > 0) {
      tolowercase(buffer);
      matchesFilter = (strstr(buffer, filter) != nullptr);
      // Trace::Log("FILESYSTEM", "FILTER: %s=%s [%d]", buffer, filter,
      //            matchesFilter);
    }
    // filter out "." and files that dont match filter if a filter is given
    if ((entry.isDirectory() && entry.dirIndex() != 0) ||
        (!entry.isHidden() && matchesFilter)) {
      if (subDirOnly) {
        if (entry.isDirectory()) {
          fileIndexes->push_back(index);
        }
      } else {
        fileIndexes->push_back(index);
      }
      // Trace::Log("FILESYSTEM", "[%d] got file: %s", index, buffer);
      count++;
    } else {
      // Trace::Log("FILESYSTEM", "skipped hidden: %s", buffer);
    }
    entry.close();
  }
  cwd.close();
  Trace::Log("FILESYSTEM", "scanned: %d, added file indexes:%d", count,
             fileIndexes->size());
}

void picoTrackerFileSystem::getFileName(int index, char *name, int length) {
  std::lock_guard<Mutex> lock(mutex);
  FsBaseFile cwd;
  if (!cwd.openCwd()) {
    char dirname[PFILENAME_SIZE];
    cwd.getName(dirname, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd:%s", dirname);
    return;
  }
  FsBaseFile entry;
  entry.open(index);
  entry.getName(name, length);
  entry.close();
  cwd.close();
}

bool picoTrackerFileSystem::isParentRoot() {
  std::lock_guard<Mutex> lock(mutex);
  FsBaseFile cwd;
  if (!cwd.openCwd()) {
    char dirname[PFILENAME_SIZE];
    cwd.getName(dirname, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd:%s", dirname);
    return false;
  }

  FsFile root;
  root.openRoot(sd.vol());
  FsFile up;
  up.open(1);
  // check the index=1 entry, aka ".." if its firstSector  matches
  // the root dirs firstSector, ie they are the same dir
  bool result = root.firstSector() == up.firstSector();
  root.close();
  up.close();
  cwd.close();
  return result;
}

bool picoTrackerFileSystem::DeleteFile(const char *path) {
  std::lock_guard<Mutex> lock(mutex);
  return sd.remove(path);
}

bool picoTrackerFileSystem::DeleteDir(const char *path) {
  std::lock_guard<Mutex> lock(mutex);
  auto delDir = sd.open(path, O_READ);
  return delDir.rmdir();
}

bool picoTrackerFileSystem::exists(const char *path) {
  std::lock_guard<Mutex> lock(mutex);
  return sd.exists(path);
}

bool picoTrackerFileSystem::makeDir(const char *path, bool pFlag) {
  std::lock_guard<Mutex> lock(mutex);
  return sd.mkdir(path, pFlag);
}

uint64_t picoTrackerFileSystem::getFileSize(const int index) {
  std::lock_guard<Mutex> lock(mutex);
  FsBaseFile cwd;
  FsBaseFile entry;
  if (!entry.open(index)) {
    char name[PFILENAME_SIZE];
    cwd.getName(name, PFILENAME_SIZE);
    Trace::Error("Failed to open file: %d", index);
  }
  auto size = entry.fileSize();
  if (size == 0) {
    size = entry.fileSize();
  }
  entry.close();
  cwd.close();
  return size;
}

bool picoTrackerFileSystem::CopyFile(const char *srcPath,
                                     const char *destPath) {
  std::lock_guard<Mutex> lock(mutex);
  auto fSrc = sd.open(srcPath, O_READ);
  auto fDest = sd.open(destPath, O_WRITE | O_CREAT);

  int n = 0;
  int bufferSize = sizeof(fileBuffer_);
  while (true) {
    n = fSrc.read(fileBuffer_, bufferSize);
    // check for read error and only write if no error
    if (n >= 0) {
      fDest.write(fileBuffer_, n);
    } else {
      Trace::Error("Failed to read file: %s", srcPath);
      return false;
    }
    if (n < bufferSize) {
      break;
    }
  }
  fSrc.close();
  fDest.close();
  return true;
}

void picoTrackerFileSystem::tolowercase(char *temp) {
  // Convert to lower case
  char *s = temp;
  while (*s != '\0') {
    *s = tolower((unsigned char)*s);
    s++;
  }
}

// picoTrackerFile implementation

picoTrackerFile::picoTrackerFile(FsBaseFile file) { file_ = file; }

int picoTrackerFile::Read(void *ptr, int size) {
  std::lock_guard<Mutex> lock(mutex);
  return file_.read(ptr, size);
}

void picoTrackerFile::Seek(long offset, int whence) {
  std::lock_guard<Mutex> lock(mutex);
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

int picoTrackerFile::GetC() {
  std::lock_guard<Mutex> lock(mutex);
  return file_.read();
}

int picoTrackerFile::Write(const void *ptr, int size, int nmemb) {
  std::lock_guard<Mutex> lock(mutex);
  return file_.write(ptr, size * nmemb);
}

long picoTrackerFile::Tell() {
  std::lock_guard<Mutex> lock(mutex);
  return file_.curPosition();
}

int picoTrackerFile::Error() {
  std::lock_guard<Mutex> lock(mutex);
  return file_.getError();
}

bool picoTrackerFile::Close() {
  std::lock_guard<Mutex> lock(mutex);
  return file_.close();
}
