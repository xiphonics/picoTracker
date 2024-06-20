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

bool PicoFileSystem::chdir(const char *name) {
  Trace::Log("PICOFILESYSTEM", "chdir:%s", name);

  sd.chvol();
  auto res = sd.vol()->chdir(name);
  File cwd;
  char buf[128];
  cwd.openCwd();
  cwd.getName(buf, 128);
  cwd.close();
  return res;
}

PicoFileType PicoFileSystem::getFileType(int index) {
  FsBaseFile cwd;
  if (!cwd.openCwd()) {
    char name[PFILENAME_SIZE];
    Trace::Error("Failed to open cwd: %s", name);
    return PFT_UNKNOWN;
  }
  FsBaseFile entry;
  entry.open(index);
  auto isDir = entry.isDirectory();
  entry.close();
  return isDir ? PFT_DIR : PFT_FILE;
}

void PicoFileSystem::list(etl::vector<int, MAX_FILE_INDEX_SIZE> *fileIndexes,
                          const char *filter) {
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

  File entry;
  int count = 0;

  // ref: https://github.com/greiman/SdFat/issues/353#issuecomment-1003422848
  while (entry.openNext(&cwd, O_READ) && (count < PFILENAME_SIZE)) {
    uint32_t index = entry.dirIndex();
    entry.getName(buffer, PFILENAME_SIZE);

    // skip hidden & "."
    bool matchesFilter = true;
    if (strlen(filter) > 0) {
      matchesFilter = (strstr(buffer, filter) != NULL);
    }
    // filter out "." and files that dont match filter if a filter is given
    if ((entry.isDirectory() && entry.dirIndex() != 0) ||
        (!entry.isHidden() && matchesFilter)) {
      fileIndexes->push_back(index);
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
  printf("ROOT:%d", result);

  root.close();
  up.close();
  cwd.close();
  return result;
}