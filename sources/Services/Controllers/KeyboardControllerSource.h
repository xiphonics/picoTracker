/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _KEYBOARD_CONTROLLER_SOURCE_H_
#define _KEYBOARD_CONTROLLER_SOURCE_H_

#include "ControllerSource.h"

#define MAX_KEY 400

class KeyboardControllerSource : public ControllerSource {
public:
  KeyboardControllerSource(const char *name);
  virtual ~KeyboardControllerSource();
  virtual Channel *GetChannel(const char *url);
  virtual bool IsRunning() { return true; };
  void SetKey(int key, bool value);

private:
  Channel *channel_[MAX_KEY];
};

#endif
