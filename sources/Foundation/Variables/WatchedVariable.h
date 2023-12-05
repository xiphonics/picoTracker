
#ifndef _WATCHED_VARIABLE_H_
#define _WATCHED_VARIABLE_H_

#include "Foundation/Observable.h"
#include "Variable.h"

class WatchedVariable : public Variable, public Observable {
public:
  WatchedVariable(const char *name, FourCC id, int value = 0);
  WatchedVariable(const char *name, FourCC id, bool value);
  WatchedVariable(const char *name, FourCC id, const char *const *list,
                  int size, int index = 0);
  virtual ~WatchedVariable(){};
  static void Enable();
  static void Disable();

protected:
  virtual void onChange();

private:
  bool updating_;
  static bool enabled_;
};
#endif
