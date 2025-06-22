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

#include "Foundation/T_SimpleList.h"
#include "SubService.h"

class Service : protected T_SimpleList<SubService> {
public:
  Service(int fourCC);
  virtual ~Service();
  virtual void Register(SubService *);
  virtual void Unregister(SubService *);
  int GetFourCC() { return fourCC_; };

private:
  int fourCC_;
};
#endif
