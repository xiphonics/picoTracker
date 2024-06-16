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

  FsFile cwd;
  // Open root directory
  if (!cwd.open("/")) {
    Trace::Error("Root dir.open failed");
  } else {
    cwd.close();
  }
}

bool PicoFileSystem::chdir(const char *name) {
  Trace::Log("PICOFILESYSTEM", "chdir:%s", name);
  auto res = sd.chdir(name);
  sd.chvol();
  File cwd;
  char buf[128];
  cwd.openCwd();
  cwd.getName(buf, 128);
  cwd.close();
  printf("====AFTER chdir:%s\n", buf);
  return res;
}

PicoFileType PicoFileSystem::getFileType(int index) {
  Trace::Log("PICOFILESYSTEM", "get file type for:%d", index);
  FsFile cwd;
  if (!cwd.openCwd()) {
    char name[PFILENAME_SIZE];
    Trace::Error("Failed to open cwd: %s", name);
    return PFT_UNKNOWN;
  }
  FsBaseFile entry;
  entry.open(index);
  auto isDir = entry.isDirectory();
  return isDir ? PFT_DIR : PFT_FILE;
}

void PicoFileSystem::list(etl::vector<int, MAX_FILE_INDEX_SIZE> *fileIndexes) {
  fileIndexes->clear();

  File cwd;
  if (!cwd.openCwd()) {
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

  FsBaseFile entry;
  int count = 0;
  // ref: https://github.com/greiman/SdFat/issues/353#issuecomment-1003422848
  while (entry.openNext(&cwd, O_READ) && (count < PFILENAME_SIZE)) {
    uint32_t index = entry.dirIndex();
    if (!entry.isHidden()) {
      fileIndexes->push_back(index);
      entry.getName(buffer, PFILENAME_SIZE);
      count++;
      printf("idx:%d file:%s\n", index, buffer);
    } else {
      entry.getName(buffer, PFILENAME_SIZE);
      Trace::Log("PICOFILESYSTEM", "skipped hidden: %s", buffer);
    }
    entry.close();
  }
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
  FsBaseFile entry;
  entry.open(index);
  entry.getName(name, length);
}
