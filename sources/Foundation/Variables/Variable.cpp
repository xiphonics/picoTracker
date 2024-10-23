#include "Variable.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"
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

Variable::Variable(FourCC id, const char *value)
    : id_(id), stringValue_(value) {
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
    sprintf(buf, "%f", value);
    stringValue_ = buf;
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
    sprintf(buf, "%d", value);
    stringValue_ = buf;
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
    stringValue_ = (value) ? "true" : "false";
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
    return float(atof(stringValue_.c_str()));
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
    return atoi(stringValue_.c_str());
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
  case CHAR_LIST:
  case INT:
    value_.int_ = atoi(string);
    break;
  case BOOL:
    value_.bool_ = (!strcmp("false", string) ? false : true);
    break;
  case STRING:
    stringValue_ = std::string(string);
    break;
  };
  if (notify) {
    onChange();
  }
};

etl::string<40> Variable::GetString() {
  char buf[40];
  switch (type_) {
  case FLOAT:
    sprintf(buf, "%f", value_.float_);
    break;
  case CHAR_LIST:
  case INT:
    sprintf(buf, "%d", value_.int_);
    break;
  case BOOL:
    strcpy(buf, value_.bool_ ? "true" : "false");
    break;
  case STRING:
    return stringValue_.c_str();
  };

  return buf;
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
    stringValue_ = "";
    break;
  }
}
