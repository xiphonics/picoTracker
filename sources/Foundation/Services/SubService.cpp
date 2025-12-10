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

SubService::SubService(int fourCC, bool registerWithService)
    : fourCC_(fourCC), registerWithService_(registerWithService) {
  if (registerWithService_) {
    ServiceRegistry::GetInstance()->Register(this);
  }
};

SubService::~SubService() {
  if (registerWithService_) {
    ServiceRegistry::GetInstance()->Unregister(this);
  }
};
