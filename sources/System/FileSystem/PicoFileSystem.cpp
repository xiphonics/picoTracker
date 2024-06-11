#include "PicoFileSystem.h"

PicoFileSystem::PicoFileSystem() {
  // Check for the common case, FAT filesystem as first partition
  Trace::Log("FILESYSTEM", "Try to mount SD Card");
  if (sd.begin(SD_CONFIG)) {
    Trace::Log("FILESYSTEM",
               "Mounted SD Card FAT Filesystem from first partition");
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

  // Open root directory
  if (!cwd.open("/")) {
    Trace::Error("dir.open failed");
  }
}

bool PicoFileSystem::chdir(const char *name) {
  Trace::Log("chdir:%s", name);
  return sd.chdir(name);
}

void PicoFileSystem::list(etl::array<int, 256> *fileIndexes) {
  fileIndexes->erase_range(fileIndexes->front(), fileIndexes->back());

  if (!cwd.openCwd()) {
    Trace::Error("Failed to open cwd");
    return;
  }

  if (!cwd.isDir()) {
    Trace::Error("Path is not a directory");
    return;
  }

  size_t count = 0;
  FsBaseFile entry;

  // ref: https://github.com/greiman/SdFat/issues/353#issuecomment-1003422848
  while (entry.openNext(&cwd, O_READ)) {
    int index = entry.dirIndex();
    fileIndexes->insert_at(count, index);
  }
  Trace::Log("PICOFILESYSTEM", "scanned %d files add entries:", count);
}

void PicoFileSystem::getFileName(int index, char *name, int length) {
  if (!cwd.openCwd()) {
    char *name[128];
    Trace::Error("Failed to open cwd");
    return;
  }
  FsBaseFile entry;
  entry.open(index);
  entry.getName(name, length);
}