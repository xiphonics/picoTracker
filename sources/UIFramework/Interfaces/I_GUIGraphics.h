/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _I_GUIGRAPHICS_H_
#define _I_GUIGRAPHICS_H_

#include "UIFramework/BasicDatas/GUIRect.h"
#include "UIFramework/Framework/GUIColor.h"
#include "UIFramework/Framework/GUITextProperties.h"
// #include "Engine/ENGBitmap.h"

// Interface definition for a graphical port.

class I_GUIGraphics {
public:
  virtual ~I_GUIGraphics(){};
  virtual void Clear(GUIColor &, bool overlay = false) = 0;
  virtual void SetColor(GUIColor &) = 0;
  virtual void ClearTextRect(GUIRect &) = 0;
  virtual void DrawString(const char *string, const GUIPoint &pos,
                          const GUITextProperties &p, bool overlay) = 0;
  virtual void DrawChar(const char c, const GUIPoint &pos,
                        const GUITextProperties &props) = 0;

  virtual GUIRect GetRect() = 0;
  virtual void Invalidate() = 0;
  virtual void Lock() = 0;
  virtual void Unlock() = 0;
  virtual void Flush() = 0;
};

#endif
