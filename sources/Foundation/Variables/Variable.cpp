/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Variable.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"
#include <System/Console/nanoprintf.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Variable::Variable(FourCC id, float value) : id_(id) {
  value_.float_ = value;
  defaultValue_.float_ = value;
  type_ = FLOAT;
};

Variable::Variable(FourCC id, int value) : id_(id) {
  value_.int_ = value;
  defaultValue_.int_ = value;
  type_ = INT;
};

Variable::Variable(FourCC id, bool value) : id_(id) {
  value_.bool_ = value;
  defaultValue_.bool_ = value;
  type_ = BOOL;
};

Variable::Variable(FourCC id, const char *value) : id_(id) {
  setStringValue(value);
  type_ = STRING;
};

Variable::Variable(FourCC id, const char *const *list, int size, int index)
    : id_(id) {
  list_.char_ = list;
  listSize_ = size;
  value_.index_ = index;
  defaultValue_.index_ = index;
  type_ = CHAR_LIST;
};

Variable::~Variable(){};

Variable::Type Variable::GetType() { return type_; };

FourCC Variable::GetID() { return id_; };

const char *Variable::GetName() { return FourCC(id_).c_str(); };

void Variable::SetFloat(float value, bool notify) {
  switch (type_) {
  case FLOAT:
    value_.float_ = value;
    break;
  case INT:
    value_.int_ = int(value);
    break;
  case BOOL:
    value_.bool_ = bool(value != 0);
    break;
  case CHAR_LIST:
    value_.index_ = int(value);
    break;
  case STRING:
    char buf[10];
    npf_snprintf(buf, sizeof(buf), "%f", value);
    setStringValue(buf);
    break;
  };
  if (notify) {
    onChange();
  }
};

void Variable::SetInt(int value, bool notify) {
  switch (type_) {
  case FLOAT:
    value_.float_ = float(value);
    break;
  case INT:
    value_.int_ = value;
    break;
  case BOOL:
    value_.bool_ = bool(value != 0);
    break;
  case CHAR_LIST:
    value_.index_ = value;
    break;
  case STRING:
    char buf[10];
    npf_snprintf(buf, sizeof(buf), "%d", value);
    setStringValue(buf);
    break;
  };
  if (notify) {
    onChange();
  }
};

void Variable::SetBool(bool value, bool notify) {
  switch (type_) {
  case FLOAT:
    value_.float_ = float(value);
    break;
  case INT:
    value_.int_ = int(value);
    break;
  case BOOL:
    value_.bool_ = value;
    break;
  case CHAR_LIST:
    value_.index_ = int(value);
    break;
  case STRING:
    setStringValue(value ? "true" : "false");
    break;
  };
  if (notify) {
    onChange();
  }
};

float Variable::GetFloat() {
  switch (type_) {
  case FLOAT:
    return value_.float_;
  case INT:
    return float(value_.int_);
  case BOOL:
    return float(value_.bool_);
  case CHAR_LIST:
    return float(value_.index_);
  case STRING:
    return float(atof(stringValue_->c_str()));
  };
  return 0.0f;
};

int Variable::GetInt() {
  switch (type_) {
  case FLOAT:
    return int(value_.float_);
  case INT:
    return value_.int_;
  case BOOL:
    return int(value_.bool_);
  case CHAR_LIST:
    return value_.index_;
  case STRING:
    return atoi(stringValue_->c_str());
  };
  return 0;
};

bool Variable::GetBool() {
  switch (type_) {
  case FLOAT:
    return bool(value_.float_ != 0);
  case INT:
    return bool(value_.int_ != 0);
  case BOOL:
    return value_.bool_;
  case CHAR_LIST:
    return bool(value_.index_ != 0);
  case STRING:
    return false;
  };
  return false;
};

void Variable::SetString(const char *string, bool notify) {
  NAssert(string);
  switch (type_) {
  case FLOAT:
    value_.float_ = float(atof(string));
    break;
  case INT:
    value_.int_ = atoi(string);
    break;
  case BOOL:
    value_.bool_ = (!strcmp("false", string) ? false : true);
    break;
  case STRING:
    setStringValue(string);
    break;
  case CHAR_LIST:
    value_.index_ = -1;
    for (int i = 0; i < listSize_; i++) {
      if (list_.char_[i]) {
        if (strcasecmp(string, list_.char_[i]) == 0) {
          value_.index_ = i;
          break;
        }
      }
    };
    break;
  };
  if (notify) {
    onChange();
  }
};

etl::string<MAX_VARIABLE_STRING_LENGTH> Variable::GetString() {
  char buf[MAX_VARIABLE_STRING_LENGTH];
  switch (type_) {
  // !!! NOTE !!! we don't want to enable nanoprintf's float support so we just
  // cast to int here because we don't really display floats anyway
  case FLOAT:
    npf_snprintf(buf, sizeof(buf), "%f", (int)value_.float_);
    break;
  case INT:
    npf_snprintf(buf, sizeof(buf), "%d", value_.int_);
    break;
  case BOOL:
    npf_snprintf(buf, sizeof(buf), "%s", value_.bool_ ? "true" : "false");
    break;
  case STRING:
    return *stringValue_;
  case CHAR_LIST:
    if ((value_.index_ < 0) || (value_.index_ >= listSize_)) {
      return "";
    } else {
      return list_.char_[value_.index_];
    }
    break;
  };

  return etl::string<MAX_VARIABLE_STRING_LENGTH>(buf, etl::strlen(buf));
};

void Variable::CopyFrom(Variable &other) {
  type_ = other.type_;
  value_ = other.value_;
  list_ = other.list_;
  listSize_ = other.listSize_;
  onChange();
}

const char *const *Variable::GetListPointer() {
  NAssert(type_ == CHAR_LIST);
  return list_.char_;
};

uint8_t Variable::GetListSize() {
  NAssert(type_ == CHAR_LIST);
  return listSize_;
};

void Variable::Reset() {

  switch (type_) {

  case FLOAT:
    value_.float_ = defaultValue_.float_;
    break;
  case INT:
    value_.int_ = defaultValue_.int_;
    break;
  case BOOL:
    value_.bool_ = defaultValue_.bool_;
    break;
  case CHAR_LIST:
    value_.index_ = defaultValue_.index_;
    break;
  case STRING:
    // TODO: Check if this may be needed in the future, not used at this time
    setStringValue("");
    break;
  }
}

void Variable::setStringValue(const char *value) {
  if (stringValue_ != nullptr) {
    delete stringValue_;
  }
  stringValue_ = new etl::string<MAX_VARIABLE_STRING_LENGTH>(value);
}

bool Variable::IsModified() {
  switch (type_) {
  case FLOAT:
    return value_.float_ != defaultValue_.float_;
  case INT:
    return value_.int_ != defaultValue_.int_;
  case BOOL:
    return value_.bool_ != defaultValue_.bool_;
  case CHAR_LIST:
    return value_.index_ != defaultValue_.index_;
  case STRING:
    // For string types, just compare against empty string
    return (stringValue_ != nullptr) && (stringValue_->size() > 0);
  }
  return false;
}
