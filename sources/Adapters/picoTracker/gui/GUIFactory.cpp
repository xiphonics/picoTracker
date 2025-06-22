/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "GUIFactory.h"
#include "picoTrackerEventManager.h"
#include "picoTrackerGUIWindowImp.h"

GUIFactory::GUIFactory(){};

I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
  alignas(picoTrackerGUIWindowImp) static char
      guiImpMemBuf[sizeof(picoTrackerGUIWindowImp)];
  return *(new (guiImpMemBuf) picoTrackerGUIWindowImp(p));
}

EventManager *GUIFactory::GetEventManager() {
  return picoTrackerEventManager::GetInstance();
}
