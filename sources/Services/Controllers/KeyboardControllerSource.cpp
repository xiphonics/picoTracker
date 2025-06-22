/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "KeyboardControllerSource.h"
#include "System/Console/Trace.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
#include <string.h>

KeyboardControllerSource::KeyboardControllerSource(const char *name)
    : ControllerSource("key", name) {
  memset(channel_, 0, MAX_KEY * sizeof(Channel *));
};

KeyboardControllerSource::~KeyboardControllerSource(){};

Channel *KeyboardControllerSource::GetChannel(const char *url) {
  int key =
      I_GUIWindowFactory::GetInstance()->GetEventManager()->GetKeyCode(url);
  if (key > 0) {
    if (!channel_[key]) {
      channel_[key] = new Channel(url);
    }
    return channel_[key];
  }
  return 0;
}

void KeyboardControllerSource::SetKey(int key, bool value) {

  if (channel_[key]) {
    channel_[key]->SetValue(value ? 1.0f : 0.0f);
    channel_[key]->NotifyObservers();
  }
}
