/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _THEME_IMPORT_VIEW_H_
#define _THEME_IMPORT_VIEW_H_

#include "ScreenView.h"
#include "System/FileSystem/FileSystem.h"
#include "ViewData.h"
#include <string>

class ThemeImportView : public ScreenView {
public:
  ThemeImportView(GUIWindow &w, ViewData *viewData);
  ~ThemeImportView();
  void Reset();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();

protected:
  void setCurrentFolder(FileSystem *fs, const char *name);
  void warpToNextTheme(bool goUp);
  void onImportTheme(const char *filename);

private:
  void onImportThemeModalDismiss(View &view, ModalView &dialog);

  size_t topIndex_ = 0;
  size_t currentIndex_ = 0;
};
#endif
