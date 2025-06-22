/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _APPLICATION_COMMAND_DISPATCHER_H_
#define _APPLICATION_COMMAND_DISPATCHER_H_

#include "Application/Model/Project.h"
#include "Foundation/T_Singleton.h"

class CommandExecuter {
public:
  CommandExecuter(){};
  virtual ~CommandExecuter(){};
  virtual void Execute(FourCC id, float value) = 0;
};

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
