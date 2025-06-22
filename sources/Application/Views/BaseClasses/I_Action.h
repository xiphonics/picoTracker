/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _I_ACTION_H_
#define _I_ACTION_H_

class I_Action {
public:
  I_Action(char *name);
  virtual ~I_Action();
  char *GetName();
  virtual void Do() = 0;

private:
  char *name_;
};

#endif
