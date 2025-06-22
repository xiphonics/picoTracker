/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _CONTROLLER_SERVICE_H_
#define _CONTROLLER_SERVICE_H_

#include "Foundation/T_SimpleList.h"
#include "Foundation/T_Singleton.h"

#include "ControllableVariable.h"
#include "ControllerSource.h"

class ControllerService : public T_Singleton<ControllerService>,
                          public T_SimpleList<ControllerSource> {
public:
  ControllerService();
  virtual ~ControllerService();
  Channel *GetChannel(const char *sourcePath);
};
#endif
