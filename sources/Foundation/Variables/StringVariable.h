/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _STRING_VARIABLE_H_
#define _STRING_VARIABLE_H_

#include "Variable.h"
#include "WatchedVariable.h"

// Lightweight string-capable variable without heap allocations.
// Stores string data in an internal ETL string buffer.
template <size_t MaxLen = MAX_VARIABLE_STRING_LENGTH>
class StringVariable : public Variable {
public:
  explicit StringVariable(FourCC id, const char *value = "")
      : Variable(id, 0) {
    type_ = STRING;
    stringValue_ = &storage_;
    setStringValue(value ? value : "");
    defaultValue_ = storage_;
  }

  void Reset() override {
    setStringValue(defaultValue_.c_str());
    onChange();
  }

  bool IsModified() override { return storage_ != defaultValue_; }

protected:
  using Variable::setStringValue;

private:
  etl::string<MaxLen> storage_;
  etl::string<MaxLen> defaultValue_;
};

template <size_t MaxLen = MAX_VARIABLE_STRING_LENGTH>
class StringWatchedVariable : public WatchedVariable {
public:
  explicit StringWatchedVariable(FourCC id, const char *value = "")
      : WatchedVariable(id, false) {
    type_ = STRING;
    stringValue_ = &storage_;
    setStringValue(value ? value : "");
    defaultValue_ = storage_;
  }

  void Reset() override {
    setStringValue(defaultValue_.c_str());
    onChange();
  }

  bool IsModified() override { return storage_ != defaultValue_; }

protected:
  using Variable::setStringValue;

private:
  etl::string<MaxLen> storage_;
  etl::string<MaxLen> defaultValue_;
};

#endif
