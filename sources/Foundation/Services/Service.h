/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "Externals/etl/include/etl/vector.h"
#include "SubService.h"

class Service {
public:
  static constexpr size_t MaxSubServices = 16;

  Service(int fourCC);
  virtual ~Service();
  virtual void Register(SubService *);
  virtual void Unregister(SubService *);
  int GetFourCC() { return fourCC_; };
  etl::vector<SubService *, MaxSubServices> &SubServices() { return subs_; }

private:
  int fourCC_;
  etl::vector<SubService *, MaxSubServices> subs_;
};
#endif
