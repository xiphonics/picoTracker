/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ButtonControllerSource.h"
#include "System/Console/Trace.h"
#include <stdlib.h>

ButtonControllerSource::ButtonControllerSource(const char *name)
    : ControllerSource("but", name){};

ButtonControllerSource::~ButtonControllerSource(){};

Channel *ButtonControllerSource::GetChannel(const char *url) {
  int button = atoi(url);
  return channel_ + button;
}

void ButtonControllerSource::SetButton(int button, bool value) {
  channel_[button].SetValue(value ? 1.0f : 0.0f);
  channel_[button].NotifyObservers();
}
