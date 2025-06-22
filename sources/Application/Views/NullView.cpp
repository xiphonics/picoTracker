/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "NullView.h"
#include <Application/AppWindow.h>
#include <nanoprintf.h>

NullView::NullView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {}

NullView::~NullView() {}

void NullView::ProcessButtonMask(unsigned short mask, bool pressed){

};

void NullView::DrawView() {

  Clear();

  GUITextProperties props;
  SetColor(CD_HILITE2);

  char buildString[SCREEN_WIDTH + 1];
  npf_snprintf(buildString, sizeof(buildString), "picoTracker build %s%s_%s",
               PROJECT_NUMBER, PROJECT_RELEASE, BUILD_COUNT);
  GUIPoint pos;
  pos._y = 22;
  pos._x = (32 - strlen(buildString)) / 2;
  DrawString(pos._x, pos._y, buildString, props);
};

void NullView::OnPlayerUpdate(PlayerEventType, unsigned int tick){

};

void NullView::OnFocus(){};
