/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Service.h"
#include "ServiceRegistry.h"
#include "System/Console/Trace.h"

Service::Service(int fourCC) {
  fourCC_ = fourCC;
  ServiceRegistry::GetInstance()->Register(this);
};

Service::~Service(){};

void Service::Register(SubService *sub) {
  if (subs_.full()) {
    Trace::Error("Service %d: subservice list full", fourCC_);
    return;
  }
  subs_.push_back(sub);
};

void Service::Unregister(SubService *sub) {
  auto it = etl::find(subs_.begin(), subs_.end(), sub);
  if (it != subs_.end()) {
    subs_.erase(it);
  }
};
