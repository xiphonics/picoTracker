/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MODAL_VIEW_H_
#define _MODAL_VIEW_H_

#include "View.h"

class ModalView : public View {
public:
  ModalView(View &);
  virtual ~ModalView();

  bool IsFinished();
  int GetReturnCode();

  void EndModal(int returnCode);
  virtual void Destroy();

protected:
  void SetWindow(int width, int height);
  virtual void ClearTextRect(int x, int y, int w, int h);
  virtual void DrawString(int x, int y, const char *txt,
                          const GUITextProperties &props);

  // Override GetAnchor to account for modal window position
  virtual GUIPoint GetAnchor();

private:
  bool finished_;
  int returnCode_;
  int left_;
  int top_;
};
#endif
