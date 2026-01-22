/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SELECTPROJECT_VIEW_H_
#define _SELECTPROJECT_VIEW_H_

#include "ScreenView.h"
#include "System/FileSystem/FileSystem.h"
#include "ViewData.h"
#include <string>

class SelectProjectView : public ScreenView {
public:
  SelectProjectView(GUIWindow &w, ViewData *viewData);
  ~SelectProjectView();
  void Reset();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();
  void getSelectedProjectName(char *name);
  void getHighlightedProjectName(char *name);
  void setCurrentFolder();
  bool SaveSelectedProject();
  void LoadProject();

protected:
  void warpToNextProject(bool goUp);

private:
  static const int numButtons_ = 3;
  size_t topIndex_ = 0;
  size_t currentIndex_ = 0;
  char selection_[MAX_PROJECT_NAME_LENGTH + 1];
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
  int selectedButton_ = 0;

  void DrawScrollBar();
  void AttemptDeletingSelectedProject();
  void AttemptLoadingProject();
  bool SelectionIsCurrentProject();
  bool WarnPlayerRunning();
  void ConfirmOverwrite();
  void SelectButton(int direction);
};
#endif
