/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _FULL_SCREEN_BOX_H_
#define _FULL_SCREEN_BOX_H_

#include "MessageBox.h"

class FullScreenBox : public MessageBox {
public:
  static FullScreenBox *Create(View &view, const char *message,
                               int btnFlags = MBBF_OK);
  static FullScreenBox *Create(View &view, const char *message,
                               const char *message2, int btnFlags = MBBF_OK);

  FullScreenBox(View &view, const char *message, int btnFlags = MBBF_OK);
  FullScreenBox(View &view, const char *message, const char *message2,
                int btnFlags = MBBF_OK);
  virtual ~FullScreenBox();
  virtual void Destroy() override;

  virtual void DrawView();
};

#endif
