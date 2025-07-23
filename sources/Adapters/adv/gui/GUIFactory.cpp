/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "GUIFactory.h"
#include "advEventManager.h"
#include "advGUIWindowImp.h"

GUIFactory::GUIFactory(){};

I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
  static char guiImpMemBuf[sizeof(advGUIWindowImp)];
  return *(new (guiImpMemBuf) advGUIWindowImp(p));
}

EventManager *GUIFactory::GetEventManager() {
  return advEventManager::GetInstance();
}
