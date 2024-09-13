#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Application/Persistency/Persistent.h"
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
  void Save();

private:
  void SaveContent(tinyxml2::XMLPrinter *printer);
  void processParams(const char *name, const char *value);
  void useDefaultConfig();

  Variable lineOut_;
};

#endif
