/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

/*
 *  EventManager.h
 *  lgpt
 *
 *  Created by Marc Nostromo on 23/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Externals/etl/include/etl/string.h"
#include "config/StringLimits.h"

#define PICO_CLOCK_INTERVAL 33 // ~30Hz
#define PICO_CLOCK_HZ (1000 / PICO_CLOCK_INTERVAL)

enum AppButton {
  APP_BUTTON_PLAY,
  APP_BUTTON_VOLINC,
  APP_BUTTON_VOLDEC,
  APP_BUTTON_LAST
};

class EventManager {
public:
  EventManager(){};
  virtual ~EventManager(){};
  virtual bool Init();
  virtual int MainLoop() = 0;
  virtual void PostQuitMessage() = 0;
  virtual int GetKeyCode(const char *name) = 0;
  void MapAppButton(const char *mapping, AppButton button);
  void InstallMappings();

protected:
  void mapConfigKey(AppButton button, const char *keyName);

private:
  etl::string<STRING_EVENT_MAPPING_MAX> mapping_[APP_BUTTON_LAST];
};
