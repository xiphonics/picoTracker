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

#include "Externals/etl/include/etl/singleton.h"
#include "Service.h"
#include "SubService.h"

class ServiceRegistryBase {
public:
  void Register(Service *);
  void Register(SubService *);
  void Unregister(SubService *);

protected:
  T_SimpleList<Service> services_;

private:
  // Only allow etl::singleton to construct
  friend class etl::singleton<ServiceRegistryBase>;
  ServiceRegistryBase(){};
};

using ServiceRegistry = etl::singleton<ServiceRegistryBase>;
#endif
