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

#include "Externals/etl/include/etl/singleton.h"
#include "Foundation/T_SimpleList.h"

#include "ControllableVariable.h"
#include "ControllerSource.h"

class ControllerServiceBase : public T_SimpleList<ControllerSource> {
public:
  Channel *GetChannel(const char *sourcePath);

private:
  // Only allow etl::singleton to construct
  friend class etl::singleton<ControllerServiceBase>;
  ControllerServiceBase(){};
};

using ControllerService = etl::singleton<ControllerServiceBase>;
#endif
