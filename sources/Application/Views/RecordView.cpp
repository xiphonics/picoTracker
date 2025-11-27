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

// Initialize static member
ViewType RecordView::sourceViewType_ = VT_SONG;

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

  position._y += 1;
  v = config->FindVariable(FourCC::VarRecordLineGain);
  intVarField_.emplace_back(position, *v, "Line gain: %d dB", LINEIN_GAIN_MINDB,
                            LINEIN_GAIN_MAXDB, 1, 2);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

  position._y += 1;
  v = config->FindVariable(FourCC::VarRecordMicGain);
  intVarField_.emplace_back(position, *v, "Mic gain: %d dB", MIC_GAIN_MINDB,
                            MIC_GAIN_MAXDB, 1, 2);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);
}

RecordView::~RecordView() {}

// Static method to set the source view type before opening SampleEditorView
void RecordView::SetSourceViewType(ViewType vt) { sourceViewType_ = vt; }

void RecordView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed) {
    return;
  }

  auto config = Config::GetInstance();

  if (mask & EPBM_NAV) {
    if (mask & EPBM_LEFT) {
      ViewType vt = sourceViewType_;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      StopMonitoring();
      config->Save();
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

      // Automatically switch to SampleEditor view after recording stops
      config->Save();
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
  pos._y += 4;

  SetColor(CD_NORMAL);

  if (isRecording_) {
    SetColor(CD_ERROR);
    DrawString(pos._x, pos._y, "[REC]", props);
    SetColor(CD_NORMAL);
  } else {
    DrawString(pos._x, pos._y, "[---]", props);
  }

  // Draw time display
  if (isRecording_) {
    SetColor(CD_ERROR);
  }
  pos._x += 6;
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

  auto config = Config::GetInstance();
#ifdef ADV
  SetLineInGain(config->FindVariable(FourCC::VarRecordLineGain)->GetInt());
  SetMicGain(config->FindVariable(FourCC::VarRecordMicGain)->GetInt());
#endif

  updateRecordingSource();
  StartMonitoring();
}

void RecordView::Update(Observable &o, I_ObservableData *data) {
  if (!hasFocus_) {
    return;
  }

  uintptr_t fourcc = (uintptr_t)data;

  auto config = Config::GetInstance();

  switch (fourcc) {
  case FourCC::VarRecordSource:
    StopMonitoring();
    updateRecordingSource();
    StartMonitoring();
    break;
#ifdef ADV
  case FourCC::VarRecordLineGain:
    SetLineInGain(config->FindVariable(FourCC::VarRecordLineGain)->GetInt());
    break;
  case FourCC::VarRecordMicGain:
    SetMicGain(config->FindVariable(FourCC::VarRecordMicGain)->GetInt());
    break;
#endif
  }
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
