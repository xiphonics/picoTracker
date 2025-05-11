#ifndef _CONTROLLER_SOURCE_H_
#define _CONTROLLER_SOURCE_H_

#include "Channel.h"
#include <string>

class ControllerSource {
public:
  ControllerSource(const char *devclass, const char *name);
  virtual ~ControllerSource();

  virtual Channel *GetChannel(const char *name) = 0;
  virtual bool IsRunning() = 0;
  const char *GetClass();
  const char *GetName();

private:
  std::string class_;
  std::string name_;
};
#endif
