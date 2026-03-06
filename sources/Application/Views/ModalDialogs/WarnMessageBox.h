/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _WARN_MESSAGE_BOX_H_
#define _WARN_MESSAGE_BOX_H_

#include "MessageBox.h"

class WarnMessageBox : public MessageBox {
public:
  static WarnMessageBox *Create(View &view, const char *message,
                                int btnFlags = MBBF_OK);
  static WarnMessageBox *Create(View &view, const char *message,
                                const char *message2, int btnFlags = MBBF_OK);
  virtual ~WarnMessageBox();
  virtual void Destroy() override;
  virtual void DrawView() override;

protected:
  WarnMessageBox(View &view, const char *message, int btnFlags = MBBF_OK);
  WarnMessageBox(View &view, const char *message, const char *message2,
                 int btnFlags = MBBF_OK);

private:
  static bool inUse_;
  static void *storage_;
};

#endif
