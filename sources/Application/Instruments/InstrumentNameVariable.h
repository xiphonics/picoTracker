/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _INSTRUMENT_NAME_VARIABLE_H_
#define _INSTRUMENT_NAME_VARIABLE_H_

#include "Foundation/Observable.h"
#include "Foundation/Variables/Variable.h"
#include "I_Instrument.h"

// A special Variable class that bridges between UITextField and I_Instrument's
// name_ field
class InstrumentNameVariable : public Variable, public Observable {
public:
  InstrumentNameVariable(I_Instrument *instrument);
  virtual ~InstrumentNameVariable();

  // Reimplement Variable methods to interact with the instrument's name
  etl::string<MAX_VARIABLE_STRING_LENGTH> GetString();
  void SetString(const char *string, bool notify = true);

private:
  I_Instrument *instrument_;
};

#endif
