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

/// Updates the animation by redrawing the battery gauge and power button UI on
/// every clock tick
void ScreenView::AnimationUpdate() {
  GUITextProperties props;
  static uint8_t batteryTick = 0;
  batteryTick++;
  if ((batteryTick % PICO_CLOCK_HZ) == 0) {
    drawBattery(props);
  }
  drawPowerButtonUI(props);
};
