/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PROJECT_VIEW_H_
#define _PROJECT_VIEW_H_

#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "BaseClasses/UITempoField.h"
#include "BaseClasses/UITextField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "ViewData.h"
#include <stdint.h>

template <uint8_t MaxLength> class UITextField;

class ProjectView : public FieldView, public I_Observer {
public:
  ProjectView(GUIWindow &w, ViewData *data);
  virtual ~ProjectView();
  void Reset();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus();

  etl::string<MAX_PROJECT_NAME_LENGTH> getProjectName() {
    return etl::string<MAX_PROJECT_NAME_LENGTH>(
        nameField_->GetString().c_str());
  };

  etl::string<MAX_PROJECT_NAME_LENGTH> getOldProjectName() {
    return oldProjName_;
  };
  void clearSaveAsFlag() {
    saveAsFlag_ = false;
    oldProjName_ = getProjectName();
  };

  // Observer for action callback

  void Update(Observable &, I_ObservableData *);

  void OnPurgeInstruments();
  void OnPurge();
  void OnQuit();

private:
  Project *project_;
  // Debug
  unsigned long lastTick_;
  unsigned long lastClock_;

  // Statically allocated field vectors
  etl::vector<UITempoField, 1> tempoField_;
  etl::vector<UIIntVarField, 4> intVarField_;
  etl::vector<UIActionField, 9> actionField_;
  etl::vector<UIStaticField, 1> staticField_;
  etl::vector<UITextField<MAX_PROJECT_NAME_LENGTH>, 1> textField_;

  // References to specific fields that need direct access
  UITextField<MAX_PROJECT_NAME_LENGTH> *nameField_;
  bool saveAsFlag_ = false;
  etl::string<MAX_PROJECT_NAME_LENGTH> oldProjName_;

  bool CanExit();
};
#endif
