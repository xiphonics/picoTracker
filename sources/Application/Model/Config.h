#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/Variables/VariableContainer.h"
#include "System/Console/Trace.h"

#define VAR_LINEOUT MAKE_FOURCC('L', 'O', 'U', 'T')

class Config : public T_Singleton<Config>, public VariableContainer {
public:
  Config();
  ~Config();
  const char *GetValue(const char *key);
  void ProcessArguments(int argc, char **argv);

private:
  Variable lineOut_;
};

#endif
