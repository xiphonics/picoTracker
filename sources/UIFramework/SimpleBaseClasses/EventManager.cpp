/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

/*
 *  EventManager.cpp
 *  lgpt
 *
 *  Created by Marc Nostromo on 23/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "EventManager.h"
#include "Application/Commands/NodeList.h"
#include "Application/Model/Config.h"
#include <cstring>

bool EventManager::Init() { return true; }

void EventManager::MapAppButton(const char *mapping, AppButton button) {

  etl::string<STRING_EVENT_MAPPING_MAX> mapString;
  if (!strchr(mapping, ':')) {
    mapString = "key:0:";
  };
  mapString += mapping;
  mapping_[button] = mapString;
};

void EventManager::InstallMappings() {
  // Controller mapping layer removed; nothing to install.
}
