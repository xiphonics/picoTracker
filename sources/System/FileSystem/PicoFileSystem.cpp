#include "PicoFileSystem.h"

PicoFileSystem::PicoFileSystem() {
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
  PI_File *wFile = 0;
  if (cwd.open(name, rmode)) {
    wFile = new PI_File(cwd);
  } else {
    Trace::Error("FILESYSTEM: Cannot open file:%s", name, mode);
  }
  return wFile;
}

bool PicoFileSystem::chdir(const char *name) {
  Trace::Log("PICOFILESYSTEM", "chdir:%s", name);

  sd.chvol();
  auto res = sd.vol()->chdir(name);
  File cwd;
  char buf[PFILENAME_SIZE];
  cwd.openCwd();
  cwd.getName(buf, 128);
  Trace::Log("PICOFILESYSTEM", "new CWD:%s\n", buf);
  cwd.close();
  return res;
}

PicoFileType PicoFileSystem::getFileType(int index) {
  FsBaseFile cwd;
  if (!cwd.openCwd()) {
    char name[PFILENAME_SIZE];
    cwd.getName(name, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd: %s", name);
    return PFT_UNKNOWN;
  }
  FsBaseFile entry;
  entry.open(index);
  auto isDir = entry.isDir();
  entry.close();
  return isDir ? PFT_DIR : PFT_FILE;
}

void PicoFileSystem::list(etl::vector<int, MAX_FILE_INDEX_SIZE> *fileIndexes,
                          const char *filter, bool subDirOnly) {
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
  Trace::Log("PICOFILESYSTEM", "LIST DIR:%s", buffer);

  if (!cwd.isDir()) {
    Trace::Error("Path is not a directory");
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
}

void PicoFileSystem::getFileName(int index, char *name, int length) {
  FsFile cwd;
  char dirname[PFILENAME_SIZE];
  if (!cwd.openCwd()) {
    cwd.getName(dirname, PFILENAME_SIZE);
    Trace::Error("Failed to open cwd:%s", dirname);
    return;
  }
  FsFile entry;
  entry.open(index);
  entry.getName(name, length);
  entry.close();
  cwd.close();
}

bool PicoFileSystem::isParentRoot() {
  FsFile cwd;
  char dirname[PFILENAME_SIZE];
  if (!cwd.openCwd()) {
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

bool PicoFileSystem::DeleteFile(const char *path) { return sd.remove(path); }

bool PicoFileSystem::exists(const char *path) { return sd.exists(path); }

bool PicoFileSystem::makeDir(const char *path) { return sd.mkdir(path); }

uint64_t PicoFileSystem::getFileSize(const int index) {
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
int PI_File::Read(void *ptr, int size) { return file_.read(ptr, size); }

void PI_File::Seek(long offset, int whence) {
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

bool PI_File::DeleteFile() { return file_.remove(); }

int PI_File::GetC() { return file_.read(); }

int PI_File::Write(const void *ptr, int size, int nmemb) {
  return file_.write(ptr, size * nmemb);
}

long PI_File::Tell() { return file_.curPosition(); }

int PI_File::Error() { return file_.getError(); }

bool PI_File::Close() { return file_.close(); }
