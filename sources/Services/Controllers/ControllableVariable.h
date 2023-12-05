
#ifndef _CONTROLLABLE_VARIABLE_H_
#define _CONTROLLABLE_VARIABLE_H_

#include "Channel.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/WatchedVariable.h"

class ControllableVariable : public WatchedVariable, I_Observer {
public:
  bool Connect(Channel &channel);
  void Disconnect();
};
#endif
