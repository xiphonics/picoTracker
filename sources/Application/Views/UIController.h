/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_CONTROLLER_H
#define _UI_CONTROLLER_H

#include "Foundation/T_Singleton.h"
#include "ViewData.h"

class UIController : public T_Singleton<UIController> {
private: // Singleton
  UIController();

public:
  static UIController *GetInstance();
  void Init(Project *, ViewData *);
  void Reset();

  // Muting functions

  void UnMuteAll();
  void ToggleMute(int from, int to);
  void SwitchSoloMode(int from, int to, bool clear);

private:
  Project *project_;
  ViewData *viewData_;

  bool soloMask_[SONG_CHANNEL_COUNT];
};
#endif
