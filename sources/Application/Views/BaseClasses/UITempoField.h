/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_TEMPOFIELD_H_
#define _UI_TEMPOFIELD_H_

#include "Foundation/Observable.h"
#include "UIIntVarField.h"

class UITempoField : public UIIntVarField, public I_Observer {
public:
  UITempoField(FourCC action, const GUIPoint &position, Variable &variable,
               const char *format, int min, int max, int xOffset, int yOffset);
  virtual void OnEditClick();
  void Update(Observable &, I_ObservableData *);
  void ProcessArrow(unsigned short mask);
  void ProcessEditArrow(unsigned short mask);

private:
  FourCC action_;
};
#endif
