/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _DEVICE_VIEW_H_
#define _DEVICE_VIEW_H_

#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UISwatchField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "ViewData.h"

class DeviceView : public FieldView, public I_Observer {
public:
  DeviceView(GUIWindow &w, ViewData *data);
  virtual ~DeviceView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus(){};

  // Observer for action callback

  void Update(Observable &, I_ObservableData *);

protected:
private:
  void addSwatchField(ColorDefinition color, GUIPoint position);

  etl::vector<UIIntVarField, 5> intVarField_;
  etl::vector<UIActionField, 2> actionField_;
  etl::vector<UIBigHexVarField, 16> bigHexVarField_;
  etl::vector<UISwatchField, 16> swatchField_;
};
#endif
