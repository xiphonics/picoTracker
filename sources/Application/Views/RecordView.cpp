/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "RecordView.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "Application/Views/SampleEditorView.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "UIController.h"
#include "ViewData.h"

#ifdef ADV
#include "Adapters/adv/audio/record.h"
#else
#include "Adapters/picoTracker/audio/record.h"
#endif

#include <cstdio>
#include <cstring>

RecordView::RecordView(GUIWindow &w, ViewData *data) : FieldView(w, data) {

  GUIPoint position = GetAnchor();

  auto config = Config::GetInstance();

  // Initialize recording state
  isRecording_ = false;
  recordingStartTime_ = 0;
  recordingDuration_ = 0;

  // Audio source selection field (Line In = 1, Mic = 2)
  Variable *v = config->FindVariable(FourCC::VarRecordSource);
  intVarField_.emplace_back(position, *v, "Audio source: %s", 1, 2, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);
}

RecordView::~RecordView() {}

void RecordView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed) {
    return;
  }

  if (mask & EPBM_NAV) {
    if (mask & EPBM_LEFT) {
      ViewType vt = VT_SONG;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      StopMonitoring();
      return;
    }
  }

  // Handle PLAY button for start/stop recording
  if (mask & EPBM_PLAY) {
    if (isRecording_) {
      stop();
      // set the current file for sample editor before switching view
      etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> filename(RECORDING_FILENAME);
      viewData_->sampleEditorFilename = filename;

      // to make sure if user "goes back" from Sample Editor they come back here
      // to Record again
      SampleEditorView::SetSourceViewType(VT_RECORD);

      // Automatically switch to SampleEditor view after recording stops
      ViewType vt = VT_SAMPLE_EDITOR;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      return;
    } else {
      record();
    }
    isDirty_ = true;
    return;
  }

  // Let FieldView handle other button presses (navigation, etc.)
  FieldView::ProcessButtonMask(mask, pressed);
}

void RecordView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title
  SetColor(CD_NORMAL);
  props.invert_ = true;
  DrawString(pos._x, pos._y, "Record", props);
  props.invert_ = false;

  // Draw recording status and time
  pos = GetAnchor();
  pos._y += 3;

  SetColor(CD_NORMAL);

  if (isRecording_) {
    SetColor(CD_ERROR);
    DrawString(pos._x, pos._y, "[REC]", props);
    SetColor(CD_NORMAL);
  } else {
    DrawString(pos._x, pos._y, "[---]", props);
  }

  // Draw time display
  pos._x += 5;
  char timeBuffer[16];
  formatTime(recordingDuration_, timeBuffer, sizeof(timeBuffer));
  DrawString(pos._x, pos._y, timeBuffer, props);

  // Draw instructions
  pos._y += 2;
  pos._x = GetAnchor()._x;
  SetColor(CD_NORMAL);
  const char *instruction =
      isRecording_ ? "PRESS PLAY TO STOP" : "PRESS PLAY TO RECORD";
  DrawString(pos._x, pos._y, instruction, props);

  // Draw fields
  FieldView::Redraw();

  SetColor(CD_NORMAL);
}

void RecordView::OnFocus() {
  isDirty_ = true;
  recordingDuration_ = 0;

  updateRecordingSource();
  StartMonitoring();
}

void RecordView::Update(Observable &o, I_ObservableData *data) {
  if (!hasFocus_) {
    return;
  }

  uintptr_t fourcc = (uintptr_t)data;

  switch (fourcc) {
  case FourCC::VarRecordSource:
    StopMonitoring();
    updateRecordingSource();
    StartMonitoring();
    break;
  }

  // Handle field updates
  isDirty_ = true;
  Config::GetInstance()->Save();
}

void RecordView::AnimationUpdate() {
  // First call the parent class implementation to draw the battery gauge, power
  // off etc
  ScreenView::AnimationUpdate();

  if (isRecording_) {
    // Update recording duration
    uint32_t currentTime = System::GetInstance()->Millis();
    recordingDuration_ = (currentTime - recordingStartTime_);
    isDirty_ = true;
    DrawView();
  }

  // Get player instance safely
  Player *player = Player::GetInstance();
  // Only process updates if we're fully initialized
  if (!player) {
    return;
  }
  GUITextProperties props;

  // Always update VU meter even if other parts of UI dont need updating
  drawMasterVuMeter(player, props);
}

void RecordView::record() {
#ifdef ADV
  if (isRecording_) {
    return;
  }

  // Get audio source setting (0 = Line In, 1 = Mic)
  auto config = Config::GetInstance();
  Variable *v = config->FindVariable(FourCC::VarRecordSource);
  int audioSource = v->GetInt();
  Trace::Log("RECORD", "Starting recording to %s, source: %s",
             RECORDING_FILENAME, audioSource == 0 ? "Line In" : "Mic");

  // Start recording with threshold and no duration set, ie. unlimited
  // recording time
  bool success = StartRecording(RECORDING_FILENAME, 10, 0);

  if (success) {
    isRecording_ = true;
    recordingStartTime_ = System::GetInstance()->Millis();
    recordingDuration_ = 0;
    Trace::Log("RECORDVIEW", "Recording started successfully");
  } else {
    Trace::Error("RECORDVIEW: Failed to start recording");
  }
#else
  Trace::Log("RECORD", "Recording not supported on pico");
#endif
}

void RecordView::stop() {
#ifdef ADV
  if (!isRecording_) {
    return;
  }

  GUITextProperties props;
  SetColor(CD_NORMAL);
  DrawString(10, 18, "Saving...", props);

  // will BLOCK until rec file is safely saved
  StopRecording();

  isRecording_ = false;
  Trace::Log("RECORD", "Recording stopped");
#endif
}

void RecordView::formatTime(uint32_t milliseconds, char *buffer,
                            size_t bufferSize) {
  uint32_t seconds = milliseconds / 1000;
  uint32_t minutes = seconds / 60;
  seconds = seconds % 60;

  snprintf(buffer, bufferSize, "%02d:%02d", (int)minutes, (int)seconds);
}

void RecordView::updateRecordingSource() {
  auto config = Config::GetInstance();
  auto source = config->FindVariable(FourCC::VarRecordSource)->GetInt();
  SetInputSource((RecordSource)source);
}
