/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SERVICE_REGISTRY_H_
#define _SERVICE_REGISTRY_H_

#include "Foundation/T_Singleton.h"
#include "Service.h"
#include "SubService.h"

class ServiceRegistry : public T_Singleton<ServiceRegistry> {
public:
  void Register(Service *);
  void Register(SubService *);
  void Unregister(SubService *);

protected:
  T_SimpleList<Service> services_;
};
#endif
