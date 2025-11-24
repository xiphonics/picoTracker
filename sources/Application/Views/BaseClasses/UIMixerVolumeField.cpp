/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UIMixerVolumeField.h"

#include "Application/AppWindow.h"
#include "ViewUtils.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"

void UIMixerVolumeField::Draw(GUIWindow &w, int offset) {
  if (src_.GetType() == Variable::INT) {
    int displayValue = src_.GetInt() + displayOffset_;
    if (displayValue >= max_) {
      GUITextProperties props;
      GUIPoint position = GetPosition();
      position._y += offset;
      char buffer[] = "][";

      if (focus_) {
        ((AppWindow &)w).SetColor(CD_HILITE2);
        props.invert_ = true;
        w.DrawString(buffer, position, props);
      } else {
        DrawLabeledField(w, position, buffer);
      }
      return;
    }
  }

  UIIntVarField::Draw(w, offset);
}

