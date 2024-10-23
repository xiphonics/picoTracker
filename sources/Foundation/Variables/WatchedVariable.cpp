
#include "WatchedVariable.h"

bool WatchedVariable::enabled_ = true;

WatchedVariable::WatchedVariable(FourCC id, int value) : Variable(id, value) {
  updating_ = false;
};

WatchedVariable::WatchedVariable(FourCC id, bool value) : Variable(id, value) {
  updating_ = false;
};

WatchedVariable::WatchedVariable(FourCC id, const char *const *list, int size,
                                 int index)
    : Variable(id, list, size, index) {
  updating_ = false;
};

void WatchedVariable::onChange() {
  if (!updating_ && enabled_) {
    updating_ = true;
    SetChanged();
    NotifyObservers();
    updating_ = false;
  }
};

void WatchedVariable::Enable() { enabled_ = true; }

void WatchedVariable::Disable() { enabled_ = false; }
