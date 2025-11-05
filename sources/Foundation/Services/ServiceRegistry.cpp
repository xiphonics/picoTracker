/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ServiceRegistry.h"

void ServiceRegistryBase::Register(Service *s) { services_.Insert(s); };

void ServiceRegistryBase::Register(SubService *s) {
  for (services_.Begin(); !services_.IsDone(); services_.Next()) {
    Service &current = services_.CurrentItem();
    if (current.GetFourCC() == s->GetFourCC()) {
      current.Register(s);
    };
  };
};

void ServiceRegistryBase::Unregister(SubService *s) {
  for (services_.Begin(); !services_.IsDone(); services_.Next()) {
    Service &current = services_.CurrentItem();
    if (current.GetFourCC() == s->GetFourCC()) {
      current.Unregister(s);
    };
  };
};
