/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _RECORD_VIEW_H_
#define _RECORD_VIEW_H_

#include "Application/Model/Config.h"
#include "Application/Views/BaseClasses/View.h"
#include "BaseClasses/UIIntVarField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "Foundation/T_SimpleList.h"
#include "ViewData.h"

class RecordView : public FieldView, public I_Observer {
public:
  RecordView(GUIWindow &w, ViewData *data);
  virtual ~RecordView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus();

  // Observer for field changes
  void Update(Observable &, I_ObservableData *);

  void AnimationUpdate() override;

protected:
private:
  // UI fields
  etl::vector<UIIntVarField, 1> intVarField_;

  // Recording state
  bool isRecording_;
  uint32_t recordingStartTime_;
  uint32_t recordingDuration_;

  // Helper methods
  void record();
  void stop();
  void updateTimeDisplay();
  void generateFullPath(etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> filename,
                        etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> *fullpath);

  // Time display helpers
  void formatTime(uint32_t milliseconds, char *buffer, size_t bufferSize);
};

#endif
