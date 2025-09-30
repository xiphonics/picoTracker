/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "Foundation/T_Singleton.h"
#include "System/FileSystem/FileSystem.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"

extern bool forceLoadUntitledProject;

class Application : public T_Singleton<Application> {

public:
  Application();
  ~Application();
  bool Init(GUICreateWindowParams &params);

  GUIWindow *GetWindow();

protected:
  bool initProject(char *projectName);
  void ensurePTDirsExist();

private:
  GUIWindow *window_;
  static Application *instance_;
  void createIfNotExists(FileSystem *fs, const char *path);
};

#endif
