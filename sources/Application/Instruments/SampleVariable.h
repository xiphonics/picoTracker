/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_VARIABLE_H_
#define _SAMPLE_VARIABLE_H_

#include "Foundation/Observable.h"
#include "Foundation/Variables/WatchedVariable.h"

class SampleVariable : public WatchedVariable, public I_Observer {
public:
  SampleVariable(FourCC id);
  ~SampleVariable();

protected:
  virtual void Update(Observable &o, I_ObservableData *d);
};
#endif
