/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_MIXER_VOLUME_FIELD_H_
#define _UI_MIXER_VOLUME_FIELD_H_

#include "UIIntVarField.h"

class UIMixerVolumeField : public UIIntVarField {
public:
  UIMixerVolumeField(GUIPoint &position, Variable &v, const char *format,
                     int min, int max, int xOffset, int yOffset,
                     int displayOffset = 0)
      : UIIntVarField(position, v, format, min, max, xOffset, yOffset,
                      displayOffset) {}

  virtual void Draw(GUIWindow &w, int offset = 0);
};

#endif
