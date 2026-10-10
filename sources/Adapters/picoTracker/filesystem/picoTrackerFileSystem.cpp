/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2024 xiphonics, inc.
 */

#include "picoTrackerFileSystem.h"
#include "Externals/etl/include/etl/pool.h"
#include <cstring>
#include <limits>
#include <utility>

Mutex mutex;
constexpr uint32_t MAX_OPEN_FILES = 10;
static etl::pool<picoTrackerFile, MAX_OPEN_FILES> filePool;

namespace {
// Reserved outside the range of FAT/exFAT directory entries. Callers retain
// compact indexes; the parent entry has no physical entry in SdFat.
constexpr int32_t ParentDirectoryIndex = std::numeric_limits<int32_t>::max();
} // namespace

picoTrackerFileSystem::picoTrackerFileSystem() {
  std::lock_guard<Mutex> lock(mutex);
  Trace::Log("FILESYSTEM", "Try to mount SD Card");
  if (sd.begin(SD_CONFIG)) {
    Trace::Log("FILESYSTEM", "Mounted SD Card first partition");
    return;
  }
  if (!sd.card() || sd.sdErrorCode() != 0) {
    Trace::Log("FILESYSTEM", "No SD Card present");
    return;
  }
  if (static_cast<FsVolume *>(&sd)->begin(sd.card(), true, 0)) {
    Trace::Log("FILESYSTEM", "Mounted SD Card without partition table");
  }
}

bool picoTrackerFileSystem::resolvePath(const char *path,
                                        picoTrackerPath::PathString &resolved) {
  if (!picoTrackerPath::Resolve(cwd_, path, resolved)) {
    Trace::Error("FILESYSTEM: Invalid or too long path: %s",
                 path ? path : "(null)");
    return false;
  }
  return true;
}

bool picoTrackerFileSystem::openDirectory(FsBaseFile &directory) {
  return directory.open(sd.vol(), cwd_, O_READ) && directory.isDir();
}

bool picoTrackerFileSystem::openEntry(int32_t index, FsBaseFile &entry) {
  if (index < 0 || index == ParentDirectoryIndex) {
    return false;
  }
  FsBaseFile directory;
  if (!openDirectory(directory)) {
    directory.close();
    return false;
  }
  const bool opened = entry.open(&directory, static_cast<uint32_t>(index));
  directory.close();
  return opened;
}

FileHandle picoTrackerFileSystem::Open(const char *name, const char *mode) {
  std::lock_guard<Mutex> lock(mutex);
  if (!mode || !*mode) {
    Trace::Error("Invalid mode: %s", mode ? mode : "(null)");
    return FileHandle();
  }
  const bool hasPlus = std::strchr(mode, '+') != nullptr;
  oflag_t flags = 0;
  switch (*mode) {
  case 'r':
    flags = hasPlus ? O_RDWR : O_RDONLY;
    break;
  case 'w':
    flags = (hasPlus ? O_RDWR : O_WRONLY) | O_CREAT | O_TRUNC;
    break;
  default:
    Trace::Error("Invalid mode: %s", mode);
    return FileHandle();
  }
  picoTrackerPath::PathString resolved;
  if (!resolvePath(name, resolved)) {
    return FileHandle();
  }
  FsBaseFile file;
  if (!file.open(sd.vol(), resolved.c_str(), flags)) {
    Trace::Error("FILESYSTEM: Cannot open file: %s", resolved.c_str());
    return FileHandle();
  }
  if (filePool.full()) {
    file.close();
    Trace::Error("FILESYSTEM: No file slots available (max %d)",
                 static_cast<int32_t>(MAX_OPEN_FILES));
    return FileHandle();
  }
  return MakeFileHandle(filePool.create(std::move(file)));
}

bool picoTrackerFileSystem::chdir(const char *path) {
  std::lock_guard<Mutex> lock(mutex);
  picoTrackerPath::PathString resolved;
  if (!resolvePath(path, resolved)) {
    return false;
  }
  FsBaseFile directory;
  const bool valid =
      directory.open(sd.vol(), resolved.c_str(), O_READ) && directory.isDir();
  directory.close();
  if (valid) {
    std::strcpy(cwd_, resolved.c_str());
    Trace::Log("FILESYSTEM", "new CWD: %s", cwd_);
  }
  return valid;
}

PicoFileType picoTrackerFileSystem::getFileType(int index) {
  std::lock_guard<Mutex> lock(mutex);
  if (index == ParentDirectoryIndex) {
    return PFT_DIR;
  }
  FsBaseFile entry;
  if (!openEntry(index, entry)) {
    return PFT_UNKNOWN;
  }
  const PicoFileType type = entry.isDir() ? PFT_DIR : PFT_FILE;
  entry.close();
  return type;
}

void picoTrackerFileSystem::list(etl::ivector<int> *fileIndexes,
                                 const char *filter, bool subDirOnly,
                                 bool includeHidden) {
  std::lock_guard<Mutex> lock(mutex);
  fileIndexes->clear();
  FsBaseFile directory;
  if (!openDirectory(directory)) {
    directory.close();
    Trace::Error("FILESYSTEM: Failed to open directory: %s", cwd_);
    return;
  }
  // Preserve the existing browser API without exposing physical dot entries.
  if (std::strcmp(cwd_, "/") != 0 && !fileIndexes->full()) {
    fileIndexes->push_back(ParentDirectoryIndex);
  }
  FsBaseFile entry;
  char name[PFILENAME_SIZE];
  while (!fileIndexes->full() && entry.openNext(&directory, O_READ)) {
    const bool isDirectory = entry.isDir();
    const bool named = entry.getName(name, sizeof(name)) != 0;
    bool matchesFilter = true;
    if (named && !isDirectory && filter != nullptr && *filter != '\0') {
      tolowercase(name);
      matchesFilter = std::strstr(name, filter) != nullptr;
    }
    if (named && (includeHidden || !entry.isHidden()) &&
        (!subDirOnly || isDirectory) && matchesFilter) {
      const uint32_t index = entry.dirIndex();
      if (index < static_cast<uint32_t>(ParentDirectoryIndex)) {
        fileIndexes->push_back(static_cast<int32_t>(index));
      }
    }
    entry.close();
  }
  entry.close();
  directory.close();
}

void picoTrackerFileSystem::getFileName(int index, char *name, int length) {
  std::lock_guard<Mutex> lock(mutex);
  if (name == nullptr || length <= 0) {
    return;
  }
  name[0] = '\0';
  if (index == ParentDirectoryIndex) {
    if (length >= 3) {
      std::strcpy(name, "..");
    }
    return;
  }
  FsBaseFile entry;
  if (openEntry(index, entry)) {
    entry.getName(name, static_cast<size_t>(length));
    entry.close();
  }
}

bool picoTrackerFileSystem::isParentRoot() {
  std::lock_guard<Mutex> lock(mutex);
  return cwd_[1] != '\0' && std::strchr(cwd_ + 1, '/') == nullptr;
}

bool picoTrackerFileSystem::isCurrentRoot() {
  std::lock_guard<Mutex> lock(mutex);
  return std::strcmp(cwd_, "/") == 0;
}

bool picoTrackerFileSystem::DeleteFile(const char *path) {
  std::lock_guard<Mutex> lock(mutex);
  picoTrackerPath::PathString resolved;
  return resolvePath(path, resolved) && sd.remove(resolved.c_str());
}

bool picoTrackerFileSystem::DeleteDir(const char *path) {
  std::lock_guard<Mutex> lock(mutex);
  picoTrackerPath::PathString resolved;
  if (!resolvePath(path, resolved)) {
    return false;
  }
  FsBaseFile directory;
  if (!directory.open(sd.vol(), resolved.c_str(), O_READ)) {
    return false;
  }
  const bool removed = directory.rmdir();
  directory.close();
  return removed;
}

bool picoTrackerFileSystem::exists(const char *path) {
  std::lock_guard<Mutex> lock(mutex);
  picoTrackerPath::PathString resolved;
  return resolvePath(path, resolved) && sd.exists(resolved.c_str());
}

bool picoTrackerFileSystem::makeDir(const char *path, bool pFlag) {
  std::lock_guard<Mutex> lock(mutex);
  picoTrackerPath::PathString resolved;
  return resolvePath(path, resolved) && sd.mkdir(resolved.c_str(), pFlag);
}

uint64_t picoTrackerFileSystem::getFileSize(const int index) {
  std::lock_guard<Mutex> lock(mutex);
  FsBaseFile entry;
  if (!openEntry(index, entry)) {
    return 0;
  }
  const uint64_t size = entry.fileSize();
  entry.close();
  return size;
}

bool picoTrackerFileSystem::CopyFile(const char *srcFilename,
                                     const char *destFilename,
                                     FileCopyProgressCallback progressCallback,
                                     void *progressContext, void *scratchBuffer,
                                     size_t scratchBufferSize) {
  std::lock_guard<Mutex> lock(mutex);
  picoTrackerPath::PathString sourcePath;
  picoTrackerPath::PathString destinationPath;
  if (!resolvePath(srcFilename, sourcePath) ||
      !resolvePath(destFilename, destinationPath)) {
    return false;
  }
  FsBaseFile fSrc;
  FsBaseFile fDest;
  if (!fSrc.open(sd.vol(), sourcePath.c_str(), O_READ)) {
    return false;
  }
  fDest.open(sd.vol(), destinationPath.c_str(), O_WRITE | O_CREAT);
  if (!fSrc || !fDest) {
    if (fSrc) {
      fSrc.close();
    }
    if (fDest) {
      fDest.close();
    }
    return false;
  }

  int n = 0;
  void *copyBuffer = fileBuffer_;
  size_t copyBufferSize = sizeof(fileBuffer_);
  if (scratchBuffer != nullptr && scratchBufferSize > 0) {
    copyBuffer = scratchBuffer;
    copyBufferSize = scratchBufferSize;
  }
  const size_t maxBufferSize =
      static_cast<size_t>(std::numeric_limits<int>::max());
  const int bufferSize = static_cast<int>(
      copyBufferSize > maxBufferSize ? maxBufferSize : copyBufferSize);
  const uint64_t totalBytes = fSrc.fileSize();
  uint64_t bytesCopied = 0;
  if (progressCallback) {
    progressCallback(0, totalBytes, progressContext);
  }
  while (true) {
    n = fSrc.read(copyBuffer, bufferSize);
    // check for read error and only write if no error
    if (n < 0) {
      Trace::Error("Failed to read file: %s", srcFilename);
      fSrc.close();
      fDest.close();
      return false;
    }
    if (n > 0 && fDest.write(copyBuffer, n) != static_cast<size_t>(n)) {
      Trace::Error("Failed to write file: %s", destFilename);
      fSrc.close();
      fDest.close();
      return false;
    }
    bytesCopied += static_cast<uint64_t>(n);
    if (progressCallback) {
      progressCallback(bytesCopied, totalBytes, progressContext);
    }
    if (n < bufferSize) {
      break;
    }
  }
  fSrc.close();
  fDest.close();
  return true;
}

bool picoTrackerFileSystem::MoveFile(const char *srcFilename,
                                     const char *destFilename) {
  std::lock_guard<Mutex> lock(mutex);
  picoTrackerPath::PathString sourcePath;
  picoTrackerPath::PathString destinationPath;
  return resolvePath(srcFilename, sourcePath) &&
         resolvePath(destFilename, destinationPath) &&
         sd.rename(sourcePath.c_str(), destinationPath.c_str());
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

picoTrackerFile::picoTrackerFile(FsBaseFile &&file)
    : file_(std::move(file)), isOpen_(true) {}

picoTrackerFile::~picoTrackerFile() { Close(); }

int picoTrackerFile::Read(void *ptr, int size) {
  std::lock_guard<Mutex> lock(mutex);
  return file_.read(ptr, size);
}

void picoTrackerFile::Seek(long offset, int whence) {
  std::lock_guard<Mutex> lock(mutex);
  switch (whence) {
  case SEEK_SET:
    file_.seekSet(offset);
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
  if (!isOpen_) {
    return true;
  }
  isOpen_ = false;
  return file_.close();
}

bool picoTrackerFile::Sync() {
  std::lock_guard<Mutex> lock(mutex);
  return file_.sync();
}

void picoTrackerFile::Dispose() {
  Close();
  std::lock_guard<Mutex> lock(mutex);
  filePool.destroy(this);
}
