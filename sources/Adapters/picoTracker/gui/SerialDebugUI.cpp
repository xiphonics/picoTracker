/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SerialDebugUI.h"
#include "Application/Model/Config.h"
#include "System/FileSystem/FileSystem.h"
#include "System/FileSystem/I_File.h"
#include "hardware/uart.h"
#include <Adapters/picoTracker/platform/gpio.h>
#include <Trace.h>
#include <cstdlib>
#include <nanoprintf.h>
#include <stdio.h>

#define ENDSTDIN 255
#define CR 13
#define BACKSPACE 8
#define DELETE 127
#define READ_BUFFER_SIZE 32

SerialDebugUI::SerialDebugUI(){};

// ref: https://forums.raspberrypi.com/viewtopic.php?t=303964
// will return true if user pressed return and the buffer now contains the
// string they entered up to pressing return
bool SerialDebugUI::readSerialIn(char *buffer, short size) {
  char chr = getchar_timeout_us(0);
  // in pico-sdk the char seemed to change from ENDSTDIN to 0xFE, no idea why
  while (chr != ENDSTDIN && chr != 0xFE) {
    // Handle backspace or delete key
    if (chr == BACKSPACE || chr == DELETE) {
      if (lp_ > 0) {
        // Move cursor back, print space, move cursor back again
        putchar('\b');
        putchar(' ');
        putchar('\b');
        lp_--; // Decrement buffer pointer
      }
    } else {
      // Regular character - echo it back and add to buffer
      putchar(chr);

      // Only add to buffer if we have space
      if (lp_ < size - 1) {
        buffer[lp_++] = chr;
      }

      // Check for carriage return (Enter key)
      if (chr == CR || lp_ == size - 1) {
        buffer[lp_] = 0; // terminate the string
        // strip off trailing carriage return char
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\r') {
          buffer[--len] = '\0';
        }
        dispatchCmd(buffer);
        lp_ = 0; // reset string buffer pointer
        return true;
      }
    }
    chr = getchar_timeout_us(0);
  }
  return false;
}

void SerialDebugUI::dispatchCmd(char *input) {
  // split input by space char
  char *cmd = strtok(input, " ");
  char *arg = strtok(NULL, " "); // get the first argument
  char *arg2 = strtok(NULL, " ");

  if (strcmp(cmd, "cat") == 0) {
    catFile(arg);
  } else if (strcmp(cmd, "ls") == 0) {
    listFiles(arg);
  } else if (strcmp(cmd, "rm") == 0) {
    rmFile(arg);
  } else if (strcmp(cmd, "save") == 0) {
    saveConfig();
  } else if (strcmp(cmd, "mkdir") == 0) {
    mkdir(arg);
  } else if (strcmp(cmd, "rmdir") == 0) {
    rmdir(arg);
  } else if (strcmp(cmd, "peek") == 0) {
    if (!arg) {
      Trace::Log("SERIALDEBUG", "Usage: peek <path> [bytes]");
    } else {
      size_t length = 64;
      if (arg2) {
        unsigned long parsed = strtoul(arg2, nullptr, 10);
        if (parsed > 0) {
          length = parsed;
        }
      }
      peekFile(arg, length);
    }
  } else if (strcmp(cmd, "help") == 0) {
    Trace::Log("SERIALDEBUG", "cat, ls, rm, mkdir, rmdir, peek, save, help");
  } else {
    Trace::Log("SERIALDEBUG", "unknown command");
  }
}

void SerialDebugUI::catFile(const char *path) {
  auto fs = FileSystem::GetInstance();
  if (fs->exists(path)) {
    auto current = fs->Open(path, "r");
    if (!current) {
      Trace::Log("SERIALDEBUG", "failed to open file:%s", path);
      return;
    }

    // Buffer for reading one character at a time
    char buffer[2] = {0};
    // Buffer for accumulating a line
    char line[READ_BUFFER_SIZE + 1] = {0};
    int linePos = 0;

    // Read the file character by character
    while (current->Read(buffer, 1) > 0) {
      // Add character to line buffer
      line[linePos++] = buffer[0];

      // If we hit a newline or buffer is full, print the line
      if (buffer[0] == '\n' || linePos >= READ_BUFFER_SIZE) {
        line[linePos] = '\0';
        printf("%s", line);
        linePos = 0;
      }
    }

    // Print any remaining characters in the buffer
    if (linePos > 0) {
      line[linePos] = '\0';
      printf("%s\n", line);
    }

    current->Close();
  } else {
    Trace::Log("SERIALDEBUG", "failed to cat file:%s", path);
  }
}

void SerialDebugUI::listFiles(const char *path) {
  auto fs = FileSystem::GetInstance();
  if (!fs->chdir(path)) {
    Trace::Error("failed to ls files path:%s", path);
  }
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexes;
  fs->list(&fileIndexes, "", false);

  char name[PFILENAME_SIZE];
  for (size_t i = 0; i < fileIndexes.size(); i++) {
    fs->getFileName(fileIndexes[i], name, PFILENAME_SIZE);
    if (fs->getFileType(fileIndexes[i]) == PFT_FILE) {
      printf("[file] %s\n", name);
    } else {
      printf("[dir] %s\n", name);
    }
  };
}

void SerialDebugUI::rmFile(const char *path) {
  auto fs = FileSystem::GetInstance();
  if (!fs->DeleteFile(path)) {
    Trace::Error("failed to delete file:%s", path);
  } else {
    Trace::Log("SERIALDEBUGUI", "deleted file:%s", path);
    char buf[128];
    npf_snprintf(buf, sizeof(buf), "deleted:%s\n", path);
    uart_write_blocking(DEBUG_UART, (uint8_t *)buf, sizeof(buf));
  }
}

void SerialDebugUI::saveConfig() {
  Config *config = Config::GetInstance();
  config->Save();
}

void SerialDebugUI::mkdir(const char *path) {
  auto fs = FileSystem::GetInstance();
  if (!fs->makeDir(path, true)) {
    Trace::Error("failed to create dir:%s", path);
  }
}

void SerialDebugUI::rmdir(const char *path) {
  auto fs = FileSystem::GetInstance();
  if (!fs->DeleteDir(path)) {
    Trace::Error("failed to remove dir:%s", path);
  }
}

void SerialDebugUI::peekFile(const char *path, size_t bytes) {
  if (!path || !*path) {
    Trace::Log("SERIALDEBUG", "Usage: peek <path> [bytes]");
    return;
  }

  auto fs = FileSystem::GetInstance();
  if (!fs->exists(path)) {
    Trace::Log("SERIALDEBUG", "File not found: %s", path);
    return;
  }

  I_File *file = fs->Open(path, "r");
  if (!file) {
    Trace::Log("SERIALDEBUG", "Failed to open file: %s", path);
    return;
  }

  if (bytes == 0) {
    bytes = 64;
  }

  uint8_t buffer[16];
  uint32_t offset = 0;
  size_t remaining = bytes;

  while (remaining > 0) {
    size_t toRead = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
    int read = file->Read(buffer, static_cast<int>(toRead));
    if (read <= 0) {
      break;
    }

    printf("%06X:", offset);
    for (int i = 0; i < read; ++i) {
      printf(" %02X", buffer[i]);
    }
    printf("\n");

    offset += static_cast<uint32_t>(read);
    remaining -= static_cast<size_t>(read);
    if (read < static_cast<int>(toRead)) {
      break;
    }
  }

  file->Close();
  delete file;
}
