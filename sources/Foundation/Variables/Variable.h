/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include "Externals/etl/include/etl/string.h"
#include "Foundation/Types/Types.h"

#define VAR_OFF -1

static const int MAX_VARIABLE_STRING_LENGTH = 40;

class Variable {

public:
  enum Type { INT, FLOAT, BOOL, CHAR_LIST, STRING };

public:
  Variable(FourCC id, int value = 0);
  Variable(FourCC id, float value = 0.0f);
  Variable(FourCC id, bool value = false);
  Variable(FourCC id, const char *value) = delete; // Use StringVariable
  Variable(FourCC id, const char *const *list, int size, int index = -1);

  virtual ~Variable();

  FourCC GetID();
  const char *GetName();

  Type GetType();
  void SetInt(int value, bool notify = true);
  int GetInt();
  void SetFloat(float value, bool notify = true);
  float GetFloat();
  virtual void SetString(const char *string, bool notify = true);
  virtual etl::string<MAX_VARIABLE_STRING_LENGTH> GetString();
  void SetBool(bool value, bool notify = true);
  bool GetBool();
  void CopyFrom(Variable &other);
  // Not very clean !
  uint8_t GetListSize();
  const char *const *GetListPointer();
  virtual void Reset();

  // Check if the current value differs from the default value
  virtual bool IsModified();

protected:
  virtual void onChange(){};
  void setStringValue(const char *value);

  FourCC id_;
  Type type_;

  union {
    int int_;
    float float_;
    bool bool_;
    int index_;
  } value_;

  union {
    int int_;
    float float_;
    bool bool_;
    int index_;
  } defaultValue_;

  union {
    const char *const *char_;
  } list_;

  etl::istring *stringValue_ = nullptr;

  uint8_t listSize_;
};
#endif
