/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _CONTROLLABLE_VARIABLE_H_
#define _CONTROLLABLE_VARIABLE_H_

#include "Channel.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/WatchedVariable.h"

class ControllableVariable : public WatchedVariable, I_Observer {
public:
  bool Connect(Channel &channel);
  void Disconnect();
};
#endif
