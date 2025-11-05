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

Service::Service(int fourCC) {
  fourCC_ = fourCC;
  ServiceRegistry::create();
  ServiceRegistry::instance().Register(this);
};

Service::~Service(){};

void Service::Register(SubService *sub) { Insert(sub); };

void Service::Unregister(SubService *sub) { Remove(*sub); };
