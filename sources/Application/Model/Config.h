#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Application/Persistency/Persistent.h"
#include "Foundation/T_Singleton.h"
#include "Foundation/Variables/VariableContainer.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "System/Console/Trace.h"

class Config : public T_Singleton<Config>, public VariableContainer {
public:
  Config();
  ~Config();
  int GetValue(const char *key);
  void ProcessArguments(int argc, char **argv);
  bool Save();

private:
  etl::list<Variable *, 24> variables_;

  void SaveContent(tinyxml2::XMLPrinter *printer);
  void processParams(const char *name, int value, bool insert);
  void useDefaultConfig();

  WatchedVariable lineOut_;
  WatchedVariable midiDevice_;
  WatchedVariable midiSync_;
  WatchedVariable remoteUI_;
  WatchedVariable uiFont_;
};

#endif
