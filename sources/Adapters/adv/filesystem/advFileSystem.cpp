/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advFileSystem.h"

FATFS SDFatFS;
char SDPath[4];

// use max int value for parent dir marker
#define PARENT_DIR_MARKER_INDEX (std::numeric_limits<int>::max())

advFileSystem::advFileSystem() {

  // Link FatFs driver
  FATFS_LinkDriver(&SD_DMA_Driver, SDPath);

  // Check for the common case, FAT filesystem as first partition
  Trace::Log("FILESYSTEM", "Try to mount SD Card");
  if (f_mount(&SDFatFS, (TCHAR const *)SDPath, 0) == FR_OK) {
    Trace::Log("FILESYSTEM", "Mounted SD Card FAT Filesystem first partition");
    return;
  }
  // Do we have any kind of card?
  // TODO: Check SD_DET pin
  //  if (!sd.card() || sd.sdErrorCode() != 0) {
  //    Trace::Log("FILESYSTEM", "No SD Card present");
  //    return;
  //  }
}

I_File *advFileSystem::Open(const char *name, const char *mode) {
  Trace::Log("FILESYSTEM", "Open file:%s, mode:%s", name, mode);
  BYTE rmode;
  switch (*mode) {
  case 'r':
    rmode = FA_READ;
    break;
  case 'w':
    rmode = FA_CREATE_ALWAYS | FA_WRITE;
    break;
  default:
    rmode = FA_READ;
    Trace::Error("Invalid mode: %s [%d]", mode, rmode);
    return 0;
  }
  FIL cwd;
  PI_File *wFile = 0;
  FRESULT res = f_open(&cwd, name, rmode);
  if (res == FR_OK) {
    wFile = new PI_File(cwd);
  } else {
    Trace::Error("FILESYSTEM: Cannot open file:%s", name, mode);
  }
  return wFile;
}

bool advFileSystem::chdir(const char *name) {
  Trace::Log("PICOFILESYSTEM", "chdir:%s", name);

  FRESULT res = f_chdir(name);
  if (res != FR_OK) {
    Trace::Error("failed to chdir into %s", name);
    return false;
  }
  char path[MAX_PROJECT_SAMPLE_PATH_LENGTH];
  res = f_getcwd(path, MAX_PROJECT_SAMPLE_PATH_LENGTH);
  if (res != FR_OK) {
    Trace::Error("failed to get current dir");
    return false;
  }
  Trace::Log("FS", "Current path is %s", path);

  DIR dir;
  FILINFO fno;
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    res = f_readdir(&dir, &fno);
  }
  f_closedir(&dir);

  return (res == FR_OK);
}

PicoFileType advFileSystem::getFileType(int index) {
  // special case for parent dir marker
  if (index == PARENT_DIR_MARKER_INDEX) {
    return PFT_DIR;
  }

  FILINFO fno;
  fno = fileFromIndex(index);
  if (fno.fattrib & AM_DIR)
    return PFT_DIR;
  return PFT_FILE;
}

void advFileSystem::list(etl::ivector<int> *fileIndexes, const char *filter,
                         bool subDirOnly) {

  fileIndexes->clear();

  // HACK: there is an assumption that ".." will be present, so add index
  // for it
  fileIndexes->push_back(PARENT_DIR_MARKER_INDEX);

  TCHAR path[PFILENAME_SIZE];
  FRESULT res = f_getcwd(path, 128);
  if (res != FR_OK) {
    Trace::Error("Directory not set");
    return;
  }
  Trace::Log("PICOFILESYSTEM", "LIST DIR:%s", path);
  DIR dir;
  res = f_opendir(&dir, path);
  if (res != FR_OK) {
    Trace::Error("Failed to open cwd");
    return;
  }

  uint32_t index = 0;
  FILINFO fno;
  uint16_t count = 0;

  while (index < fileIndexes->capacity()) {
    index = count;
    count++;
    res = f_readdir(&dir, &fno); // Read a directory item
    if (res != FR_OK || fno.fname[0] == 0)
      break; // Error or end of dir

    bool matchesFilter = true;
    if (strlen(filter) > 0) {
      tolowercase(fno.fname);
      matchesFilter = (strstr(fno.fname, filter) != nullptr);
      Trace::Log("PICOFILESYSTEM", "FILTER: %s=%s [%d]\n", fno.fname, filter,
                 matchesFilter);
    }

    // filter out "." and files that dont match filter if a filter is given
    if (!(fno.fattrib & AM_HID) || matchesFilter) {
      if (subDirOnly) {
        if (fno.fattrib & AM_DIR) {
          fileIndexes->push_back(index);
        }
      } else {
        fileIndexes->push_back(index);
      }
      Trace::Log("PICOFILESYSTEM", "[%d] got file: %s", index, fno.fname);
    } else {
      Trace::Log("PICOFILESYSTEM", "skipped hidden: %s", fno.fname);
    }
  }
  f_closedir(&dir);
  Trace::Log("PICOFILESYSTEM", "scanned: %d, added file indexes:%d", count,
             fileIndexes->size());
}

void advFileSystem::getFileName(int index, char *name, int length) {
  // special case for parent dir marker
  if (index == PARENT_DIR_MARKER_INDEX) {
    strcpy(name, "..");
    return;
  }
  FILINFO fno = fileFromIndex(index);
  strcpy(name, fno.fname);
}

FILINFO advFileSystem::fileFromIndex(int index) {
  FILINFO fno;
  FRESULT res = f_getcwd(filepath, 256);
  int32_t count = 0;
  if (res == FR_OK) {
    DIR dir;
    res = f_opendir(&dir, filepath);
    if (res == FR_OK) {
      for (;;) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0)
          break;
        if (count == index) {
          f_closedir(&dir);
          return fno;
        }
        count++;
      }
    }
    f_closedir(&dir);
  }
  return fno;
}

bool advFileSystem::isParentRoot() {
  FRESULT res = f_getcwd(filepath, sizeof(filepath));
  if (res != FR_OK) {
    Trace::Error("Failed to get current directory");
    return false;
  }

  // If current path is root ("/"), then parent is also root
  if (strcmp(filepath, "/") == 0) {
    return true;
  }

  // Check if we're in a direct subdirectory of root
  // Count the number of '/' characters - if only 1, we're in root's child
  int slashCount = 0;
  for (int i = 0; filepath[i] != '\0'; i++) {
    if (filepath[i] == '/') {
      slashCount++;
    }
  }
  // If path is "/dirname", slashCount will be 1, meaning parent is root
  return (slashCount == 1);
}

bool advFileSystem::DeleteFile(const char *path) {
  FILINFO fil;
  FRESULT res;
  res = f_stat(path, &fil);
  if (res != FR_OK) {
    Trace::Error("Path does not exist");
    return false;
  }
  if (fil.fattrib & AM_DIR) {
    Trace::Error("Path is not a file");
    return false;
  }
  res = f_unlink(path);
  return res == FR_OK;
}

// directory has to be empty
bool advFileSystem::DeleteDir(const char *path) {
  FILINFO fil;
  FRESULT res;
  res = f_stat(path, &fil);
  if (res != FR_OK) {
    Trace::Error("Path does not exist");
    return false;
  }
  if (!(fil.fattrib & AM_DIR)) {
    Trace::Error("Path is not a directory");
    return false;
  }
  res = f_unlink(path);
  return res == FR_OK;
}

bool advFileSystem::exists(const char *path) {
  FILINFO fno;
  FRESULT res = f_stat(path, &fno);
  return res == FR_OK;
}

/**
 * Create a directory at the specified path.
 *
 * \param[in] path The path where the directory will be created.
 * \param[in] pFlag If true, create missing parent directories.
 *
 * \return true if the directory was successfully created, false otherwise.
 */
bool advFileSystem::makeDir(const char *path, bool pFlag) {
  if (!pFlag) {
    FRESULT res = f_mkdir(path);
    return res == FR_OK;
  }

  char tempPath[256];
  strncpy(tempPath, path, sizeof(tempPath));
  tempPath[sizeof(tempPath) - 1] = '\0'; // ensure null-termination

  size_t len = strlen(tempPath);
  if (len == 0)
    return false;

  // Iterate through the path and create components
  for (size_t i = 1; i < len; ++i) {
    if (tempPath[i] == '/') {
      tempPath[i] = '\0';

      FRESULT res = f_mkdir(tempPath);
      if (res != FR_OK && res != FR_EXIST) {
        return false;
      }

      tempPath[i] = '/'; // restore slash
    }
  }

  // Make final directory
  FRESULT res = f_mkdir(tempPath);
  return (res == FR_OK || res == FR_EXIST);
}

uint64_t advFileSystem::getFileSize(const int index) {
  FILINFO fno = fileFromIndex(index);

  return fno.fsize;
}

bool advFileSystem::CopyFile(const char *srcPath, const char *destPath) {
  FIL fsrc, fdst; // File objects
  UINT br, bw;    // File read/write count
  FRESULT res;
  // Open source file on the drive 1
  res = f_open(&fsrc, srcPath, FA_READ);
  if (res != FR_OK)
    return false;

  // Create destination file on the drive 0
  res = f_open(&fdst, destPath, FA_WRITE | FA_CREATE_ALWAYS);
  if (res != FR_OK)
    return false;

  // Copy source to destination
  for (;;) {
    res = f_read(&fsrc, fileBuffer_, sizeof(fileBuffer_),
                 &br); // Read a chunk of data from the source file
    if (br == 0)
      break; // error or eof
    res = f_write(&fdst, fileBuffer_, br,
                  &bw); // Write it to the destination file
    if (bw < br)
      break; // error or disk full
  }

  // Close open files
  f_close(&fsrc);
  f_close(&fdst);

  return res == FR_OK;
}

void advFileSystem::tolowercase(char *temp) {
  // Convert to upper case
  char *s = temp;
  while (*s != '\0') {
    *s = tolower((unsigned char)*s);
    s++;
  }
}

PI_File::PI_File(FIL file) { file_ = file; };

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
  UINT read;
  FRESULT res = f_read(&file_, ptr, size, &read);
  return read;
}

void PI_File::Seek(long offset, int whence) {
  FRESULT res;
  UNUSED(res);
  switch (whence) {
  case SEEK_SET:
    res = f_lseek(&file_, offset);
    break;
  case SEEK_CUR:
    res = f_lseek(&file_, f_tell(&file_) + offset);
    break;
  case SEEK_END:
    res = f_lseek(&file_, f_size(&file_));
    break;
  default:
    Trace::Error("Invalid seek whence: %s", whence);
  }
}

int PI_File::GetC() {
  TCHAR c[2];
  f_gets(c, 2, &file_);
  return c[0];
}

int PI_File::Write(const void *ptr, int size, int nmemb) {
  UINT written;
  FRESULT res = f_write(&file_, ptr, size * nmemb, &written);
  return written;
}

long PI_File::Tell() { return f_tell(&file_); }

int PI_File::Error() { return f_error(&file_); }

bool PI_File::Close() {
  FRESULT res = f_close(&file_);
  return res == FR_OK;
}

bool PI_File::Sync() {
  FRESULT res = f_sync(&file_);
  return res == FR_OK;
}
