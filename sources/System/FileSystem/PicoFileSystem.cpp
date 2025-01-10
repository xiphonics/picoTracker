#include "PicoFileSystem.h"

#include "pico/multicore.h"

semaphore_t fileAccessSem;

PicoFileSystem::PicoFileSystem() {
  // init out access mutex
  sem_init(&fileAccessSem, 1, 1);

  auto available = sem_available(&fileAccessSem);
  Trace::Log("FILESYSTEM", "semaphore available: %d", available);

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
    Trace::Log("PICOFILESYSTEM",
               "Mounted SD Card FAT Filesystem without partition table");
    return;
  }
}

PI_File *PicoFileSystem::Open(const char *name, const char *mode) {
  Trace::Log("FILESYSTEM", "Open file:%s, mode:%s", name, mode);
  lockAccess();
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
    unlockAccess();
    return 0;
  }
  FsBaseFile cwd;
  if (!cwd.openCwd()) {
    unlockAccess();
    return nullptr;
  }
  PI_File *wFile = 0;
  if (cwd.open(name, rmode)) {
    wFile = new PI_File(cwd);
  } else {
    Trace::Error("FILESYSTEM: Cannot open file:%s", name, mode);
  }
  unlockAccess();
  return wFile;
}

bool PicoFileSystem::chdir(const char *name) {
  Trace::Log("PICOFILESYSTEM", "chdir:%s", name);
  lockAccess();

  sd.chvol();
  auto res = sd.vol()->chdir(name);
  File cwd;
  char buf[PFILENAME_SIZE];
  cwd.openCwd();
  cwd.getName(buf, 128);
  Trace::Log("PICOFILESYSTEM", "new CWD:%s\n", buf);
  cwd.close();

  unlockAccess();
  return res;
}

PicoFileType PicoFileSystem::getFileType(int index) {
  lockAccess();

  FsBaseFile cwd;
  if (!cwd.openCwd()) {
    char name[PFILENAME_SIZE];
    cwd.getName(name, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd: %s", name);
    unlockAccess();
    return PFT_UNKNOWN;
  }
  FsBaseFile entry;
  entry.open(index);
  auto isDir = entry.isDirectory();
  entry.close();

  unlockAccess();
  return isDir ? PFT_DIR : PFT_FILE;
}

void PicoFileSystem::list(etl::vector<int, MAX_FILE_INDEX_SIZE> *fileIndexes,
                          const char *filter, bool subDirOnly) {
  lockAccess();

  fileIndexes->clear();

  File cwd;
  if (!cwd.openCwd()) {
    char name[PFILENAME_SIZE];
    cwd.getName(name, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd");
    unlockAccess();
    return;
  }
  char buffer[PFILENAME_SIZE];
  cwd.getName(buffer, PFILENAME_SIZE);
  Trace::Log("PICOFILESYSTEM", "LIST DIR:%s", buffer);

  if (!cwd.isDir()) {
    Trace::Error("Path is not a directory");
    unlockAccess();
    return;
  }

  File entry;
  int count = 0;

  // ref: https://github.com/greiman/SdFat/issues/353#issuecomment-1003422848
  while (entry.openNext(&cwd, O_READ) && (count < PFILENAME_SIZE)) {
    uint32_t index = entry.dirIndex();
    entry.getName(buffer, PFILENAME_SIZE);

    bool matchesFilter = true;
    if (strlen(filter) > 0) {
      tolowercase(buffer);
      matchesFilter = (strstr(buffer, filter) != nullptr);
      Trace::Log("PICOFILESYSTEM", "FILTER: %s=%s [%d]\n", buffer, filter,
                 matchesFilter);
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
      Trace::Log("PICOFILESYSTEM", "[%d] got file: %s", index, buffer);
      count++;
    } else {
      Trace::Log("PICOFILESYSTEM", "skipped hidden: %s", buffer);
    }
    entry.close();
  }
  cwd.close();
  Trace::Log("PICOFILESYSTEM", "scanned: %d, added file indexes:%d", count,
             fileIndexes->size());

  unlockAccess();
}

void PicoFileSystem::getFileName(int index, char *name, int length) {
  lockAccess();

  FsFile cwd;
  char dirname[PFILENAME_SIZE];
  if (!cwd.openCwd()) {
    cwd.getName(dirname, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd:%s", dirname);
    unlockAccess();
    return;
  }
  FsFile entry;
  entry.open(index);
  entry.getName(name, length);
  entry.close();
  cwd.close();

  unlockAccess();
}

bool PicoFileSystem::isParentRoot() {
  lockAccess();

  FsFile cwd;
  char dirname[PFILENAME_SIZE];
  if (!cwd.openCwd()) {
    cwd.getName(dirname, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd:%s", dirname);
    unlockAccess();
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

  unlockAccess();
  return result;
}

bool PicoFileSystem::DeleteFile(const char *path) {
  lockAccess();
  bool result = sd.remove(path);
  unlockAccess();
  return result;
}

bool PicoFileSystem::exists(const char *path) {
  lockAccess();
  bool result = sd.exists(path);
  unlockAccess();
  return result;
}

bool PicoFileSystem::makeDir(const char *path) {
  lockAccess();
  bool result = sd.mkdir(path);
  unlockAccess();
  return result;
}

uint64_t PicoFileSystem::getFileSize(const int index) {
  lockAccess();
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
  unlockAccess();
  return size;
}

void PicoFileSystem::tolowercase(char *temp) {
  // Convert to upper case
  char *s = temp;
  while (*s != '\0') {
    *s = tolower((unsigned char)*s);
    s++;
  }
}

inline void PicoFileSystem::lockAccess() {
  // Trace::Log("PICOFILESYSTEM", "lockAccess");
  sem_acquire_blocking(&fileAccessSem);
}

inline void PicoFileSystem::unlockAccess() {
  // Trace::Log("PICOFILESYSTEM", "unlockAccess");
  sem_release(&fileAccessSem);
}

PI_File::PI_File(FsBaseFile file) { file_ = file; };

/**
 * Read data from a file starting at the current position.
 *
 * \param[out] buf Pointer to the location that will receive the data.
 *
 * \param[in] size Maximum number of bytes to read.
 *
 * \return For success read() returns the number of bytes read.
 * A value less than \a count, including zero, will be returned
 * if end of file is reached.
 * If an error occurs, read() returns -1.  Possible errors include
 * read() called before a file has been opened, corrupt file system
 * or an I/O error occurred.
 */
int PI_File::Read(void *ptr, int size) {
  PicoFileSystem::lockAccess();
  int res = file_.read(ptr, size);
  PicoFileSystem::unlockAccess();
  return res;
}

void PI_File::Seek(long offset, int whence) {
  PicoFileSystem::lockAccess();
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
  PicoFileSystem::unlockAccess();
}

bool PI_File::DeleteFile() {
  PicoFileSystem::lockAccess();
  bool res = file_.remove();
  PicoFileSystem::unlockAccess();
  return res;
}

int PI_File::GetC() {
  PicoFileSystem::lockAccess();
  int res = file_.read();
  PicoFileSystem::unlockAccess();
  return res;
}

int PI_File::Write(const void *ptr, int size, int nmemb) {
  PicoFileSystem::lockAccess();
  int res = file_.write(ptr, size * nmemb);
  PicoFileSystem::unlockAccess();
  return res;
}

long PI_File::Tell() {
  PicoFileSystem::lockAccess();
  long res = file_.curPosition();
  PicoFileSystem::unlockAccess();
  return res;
}

int PI_File::Error() {
  PicoFileSystem::lockAccess();
  int res = file_.getError();
  PicoFileSystem::unlockAccess();
  return res;
}

bool PI_File::Close() {
  PicoFileSystem::lockAccess();
  bool res = file_.close();
  PicoFileSystem::unlockAccess();
  return res;
}