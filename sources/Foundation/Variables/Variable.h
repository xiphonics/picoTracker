#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include "Externals/etl/include/etl/string.h"
#include "Foundation/Types/Types.h"

#define VAR_OFF -1
#include <string>

static const int MAX_VARIABLE_STRING_LENGTH = 40;

class Variable {

public:
  enum Type { INT, FLOAT, BOOL, CHAR_LIST, STRING };

public:
  Variable(FourCC id, int value = 0);
  Variable(FourCC id, float value = 0.0f);
  Variable(FourCC id, bool value = false);
  Variable(FourCC id, const char *value = 0);
  Variable(FourCC id, const char *const *list, int size, int index = -1);

  virtual ~Variable();

  FourCC GetID();
  const char *GetName();

  Type GetType();
  void SetInt(int value, bool notify = true);
  int GetInt();
  void SetFloat(float value, bool notify = true);
  float GetFloat();
  void SetString(const char *string, bool notify = true);
  etl::string<40> GetString();
  void SetBool(bool value, bool notify = true);
  bool GetBool();
  void CopyFrom(Variable &other);
  // Not very clean !
  uint8_t GetListSize();
  const char *const *GetListPointer();
  void Reset();

protected:
  virtual void onChange(){};

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

  etl::string<MAX_VARIABLE_STRING_LENGTH> stringValue_;

  uint8_t listSize_;
};
#endif
