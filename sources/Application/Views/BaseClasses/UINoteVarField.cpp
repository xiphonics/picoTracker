/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UINoteVarField.h"
#include "Application/AppWindow.h"
#include "Application/Utils/char.h"
#include "ViewUtils.h"
#include <System/Console/nanoprintf.h>

UINoteVarField::UINoteVarField(GUIPoint &position, Variable &v,
                               const char *format, int min, int max,
                               int xOffset, int yOffset)
    : UIIntVarField(position, v, format, min, max, xOffset, yOffset){};

void UINoteVarField::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position = GetPosition();
  position._y += offset;

  char buffer[MAX_FIELD_WIDTH + 1];
  char note[5];

  unsigned char pitch = src_.GetInt();
  note2char(pitch, note);
  note[4] = 0;
  npf_snprintf(buffer, sizeof(buffer), format_, note);

  if (focus_) {
    ((AppWindow &)w).SetColor(CD_HILITE2);
    props.invert_ = true;
    w.DrawString(buffer, position, props);
  } else {
    DrawLabeledField(w, position, buffer);
  }
};
