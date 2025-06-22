/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ScreenView.h"
#include <Application/AppWindow.h>
#include <nanoprintf.h>

ScreenView::ScreenView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {}

ScreenView::~ScreenView() {}

/// Updates the animation by redrawing the battery gauge on every clock tick
void ScreenView::AnimationUpdate() {
  GUITextProperties props;
  drawBattery(props);

  // Flush the window to ensure the battery gauge is displayed
  w_.Flush();
};