/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "System/Console/Trace.h"
#include <string>

class Result {
public:
  Result(const std::string &error);
  Result(const std::ostringstream &error);
  Result(Result &cause, const std::string &error);
  ~Result();
  Result(const Result &other);

  bool Failed();
  bool Succeeded();
  std::string GetDescription();

  Result &operator=(const Result &);

  static Result NoError;

protected:
  Result();

private:
  std::string error_;
  bool success_;
  mutable bool checked_;
  mutable Result *child_;
};

#define RETURN_IF_FAILED_MESSAGE(r, m)                                         \
  if (r.Failed()) {                                                            \
    return Result(r, m);                                                       \
  }
#define LOG_IF_FAILED(r, m)                                                    \
  if (r.Failed()) {                                                            \
    Trace::Log(m);                                                             \
    Trace::Log(r.GetDescription().c_str());                                    \
  }
