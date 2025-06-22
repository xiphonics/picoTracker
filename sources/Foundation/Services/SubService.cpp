/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SubService.h"
#include "ServiceRegistry.h"

SubService::SubService(int fourCC) {
  fourCC_ = fourCC;
  ServiceRegistry::GetInstance()->Register(this);
};

SubService::~SubService() { ServiceRegistry::GetInstance()->Unregister(this); };
