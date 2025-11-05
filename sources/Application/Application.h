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

#include "Externals/etl/include/etl/singleton.h"
#include "System/FileSystem/FileSystem.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"

extern bool forceLoadUntitledProject;

class ApplicationBase {

public:
  bool Init(GUICreateWindowParams &params);

  GUIWindow *GetWindow();

protected:
  bool initProject(char *projectName);
  void ensurePTDirsExist();

private:
  // Only allow etl::singleton to construct
  friend class etl::singleton<ApplicationBase>;
  ApplicationBase(){};

  GUIWindow *window_;
  static ApplicationBase *instance_;
  void createIfNotExists(FileSystem *fs, const char *path);
};

using Application = etl::singleton<ApplicationBase>;
#endif
