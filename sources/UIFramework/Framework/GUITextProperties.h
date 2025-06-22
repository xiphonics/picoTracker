/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _GUI_TEXT_PROPERTIES_H_
#define _GUI_TEXT_PROPERTIES_H_

struct GUITextProperties {
  GUITextProperties() : invert_(false){};
  bool invert_;
};

#endif
