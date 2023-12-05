#ifndef _APPLICATION_COMMAND_DISPATCHER_H_
#define _APPLICATION_COMMAND_DISPATCHER_H_

#include "Application/Model/Project.h"
#include "CommandDispatcher.h"
#include "Foundation/T_Singleton.h"

class ApplicationCommandDispatcher
    : public T_Singleton<ApplicationCommandDispatcher>,
      public CommandExecuter {
public:
  ApplicationCommandDispatcher();
  ~ApplicationCommandDispatcher();
  void Init(Project *project);
  void Close();
  virtual void Execute(FourCC id, float value);
  void OnTempoTap();
  void OnQueueRow();
  void OnNudgeDown();
  void OnNudgeUp();

private:
  Project *project_;
};

#endif