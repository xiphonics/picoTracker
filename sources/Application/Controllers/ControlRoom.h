/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _CONTROLROOM_H_
#define _CONTROLROOM_H_

#include "Foundation/T_Singleton.h"
#include "Services/Controllers/ControlNode.h"

class ControlRoom : public T_Singleton<ControlRoom>, public ControlNode {
public:
  ControlRoom();
  ~ControlRoom();

  bool Init();
  void Close();

  bool Attach(const char *nodeUrl, const char *controllerUrl);
  AssignableControlNode *
  GetControlNode(const etl::string<STRING_CONTROL_PATH_MAX> &url);

  void Dump();
};
#endif
