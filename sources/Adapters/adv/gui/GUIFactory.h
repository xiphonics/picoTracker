/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef GUI_FACTORY_H_
#define GUI_FACTORY_H_

#include "UIFramework/Interfaces/I_GUIWindowFactory.h"

class GUIFactory : public I_GUIWindowFactory {

public:
  GUIFactory();
  virtual I_GUIWindowImp &CreateWindowImp(GUICreateWindowParams &);
  virtual EventManager *GetEventManager();
};

#endif
