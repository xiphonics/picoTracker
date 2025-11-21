/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef SERIAL_DEBUG_UI_H_
#define SERIAL_DEBUG_UI_H_

#include <cstddef>

class SerialDebugUI {
public:
  SerialDebugUI();
  bool readSerialIn(char *buffer, short size);
  void dispatchCmd(char *cmd);
  void catFile(const char *path);
  void listFiles(const char *path);
  void rmFile(const char *path);
  void saveConfig();
  void mkdir(const char *path);
  void rmdir(const char *path);
  void shutdown();
  void readBattery();
  void peekFile(const char *path, size_t bytes);

private:
  int lp_ = 0;
};

#endif
