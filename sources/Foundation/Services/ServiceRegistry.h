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

#include "Externals/etl/include/etl/vector.h"
#include "Foundation/T_Singleton.h"
#include "Service.h"
#include "SubService.h"

class ServiceRegistry : public T_Singleton<ServiceRegistry> {
public:
  static constexpr size_t MaxServices = 16;

  void Register(Service *);
  void Register(SubService *);
  void Unregister(SubService *);
  etl::vector<Service *, MaxServices> &Services() { return services_; }

protected:
  etl::vector<Service *, MaxServices> services_;
};
#endif
