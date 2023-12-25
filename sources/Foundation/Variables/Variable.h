#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include "Foundation/Types/Types.h"

#define VAR_OFF -1
#include <string>

class Variable {

public:
  enum Type { INT, FLOAT, BOOL, CHAR_LIST, STRING };

public:
  Variable(const char *name, FourCC id, int value = 0);
  Variable(const char *name, FourCC id, float value = 0.0f);
  Variable(const char *name, FourCC id, bool value = false);
  Variable(const char *name, FourCC id, const char *value = nullptr);
  Variable(const char *name, FourCC id, const char *const *list, int8_t size = -1, int8_t index = -1);

  virtual ~Variable();

  FourCC GetID();
  char *GetName();

  Type GetType();
  void SetInt(int value, bool notify = true);
  int GetInt();
  void SetFloat(float value, bool notify = true);
  float GetFloat();
  void SetString(const char *string, bool notify = true);
  const char *GetString();
  void SetBool(bool value, bool notify = true);
  bool GetBool();
  void CopyFrom(Variable &other);
  // Not very clean !
  int GetListSize();
  const char *const *GetListPointer();
  void Reset();

protected:
  virtual void onChange(){};

  char name_[40];
  FourCC id_;
  Type type_;

  union {
    uint8_t int_;
    float float_;
    bool bool_;
    uint8_t index_;
  } value_;

  union {
    uint8_t int_;
    float float_;
    bool bool_;
    uint8_t index_;
  } defaultValue_;

  char stringValue_[40];
  char stringDefaultValue_[40];

  uint8_t listSize_;

  char string_[40];

  union {
    const char *const *char_;
  } list_;
};
#endif
