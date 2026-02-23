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
#include "System/FileSystem/FileSystem.h"
#include "System/System/System.h"
#include "UIController.h"
#include "ViewData.h"

#ifdef ADV
#include "Adapters/adv/audio/record.h"
#else
#include "Adapters/picoTracker/audio/record.h"
#endif

static constexpr uint32_t kMaxRecordDurationMs = 30000;

// Initialize static member
ViewType RecordView::sourceViewType_ = VT_SONG;

RecordView::RecordView(GUIWindow &w, ViewData *data) : FieldView(w, data) {

  GUIPoint position = GetAnchor();

  auto config = Config::GetInstance();

  // Initialize recording state
  uiRecordingActive_ = false;
  uiSavingActive_ = false;
  autoSwitchPending_ = false;
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

void RecordView::Reset() {
  uiRecordingActive_ = false;
  uiSavingActive_ = false;
  autoSwitchPending_ = false;
  recordingStartTime_ = 0;
  recordingDuration_ = 0;
}

// Static method to set the source view type before opening SampleEditorView
void RecordView::SetSourceViewType(ViewType vt) { sourceViewType_ = vt; }

void RecordView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed) {
    return;
  }

  if (uiSavingActive_) {
    // Ignore input while saving to keep stop/save flow deterministic.
    return;
  }

  // While actively recording, lock out all parameter edits/navigation except:
  // - PLAY: stop recording (existing save/switch flow)
  // - NAV-LEFT: leave immediately and stop recording in background
  if (uiRecordingActive_) {
    if ((mask & EPBM_NAV) && (mask & EPBM_LEFT)) {
#ifdef ADV
      RequestStopRecording();
#endif
      uiRecordingActive_ = false;
      uiSavingActive_ = false;
      autoSwitchPending_ = false;

      ViewType vt = sourceViewType_;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      return;
    }

    if (mask & EPBM_PLAY) {
      stopAndSwitchToEditor();
      return;
    }

    // Only disable editing of the audio source selector while recording.
    // Allow navigation and edits for the gain fields.
    if (!intVarField_.empty() && GetFocus() == &intVarField_[0] &&
        (mask & (EPBM_ENTER | EPBM_EDIT))) {
      return;
    }

    FieldView::ProcessButtonMask(mask, pressed);
    return;
  }

  if (mask & EPBM_NAV) {
    if (mask & EPBM_LEFT) {
      ViewType vt = sourceViewType_;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      StopMonitoring();
      return;
    }
  }

  // Handle PLAY button for start/stop recording
  if (mask & EPBM_PLAY) {
    record();
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

  if (uiSavingActive_) {
    SetColor(CD_ERROR);
    DrawString(pos._x, pos._y, "SAVING", props);
    SetColor(CD_NORMAL);
  } else if (uiRecordingActive_) {
    SetColor(CD_ERROR);
    DrawString(pos._x, pos._y, "[REC]", props);
    SetColor(CD_NORMAL);
  } else {
    DrawString(pos._x, pos._y, "[---]", props);
  }

  // Draw time display
  if (uiRecordingActive_ || uiSavingActive_) {
    SetColor(CD_ERROR);
  }
  pos._x += 7;
  if (uiSavingActive_) {
    char percentBuffer[8];
    uint8_t percent = GetSavingProgressPercent();
    snprintf(percentBuffer, sizeof(percentBuffer), "%3u%%", percent);
    DrawString(pos._x, pos._y, percentBuffer, props);
  } else {
    char timeBuffer[16];
    formatTime(recordingDuration_, timeBuffer, sizeof(timeBuffer));
    DrawString(pos._x, pos._y, timeBuffer, props);
  }

  // Draw instructions
  pos._y += 2;
  pos._x = GetAnchor()._x;
  SetColor(CD_NORMAL);
  const char *instruction = uiSavingActive_      ? ""
                            : uiRecordingActive_ ? "PRESS PLAY TO STOP"
                                                 : "PRESS PLAY TO RECORD";
  DrawString(pos._x, pos._y, instruction, props);

  // Draw fields. While recording, render the audio source selector without
  // focus highlight so it appears disabled.
  FieldView::Redraw();
  if (uiRecordingActive_ && !intVarField_.empty() && GetFocus() == &intVarField_[0]) {
    intVarField_[0].ClearFocus();
    intVarField_[0].Draw(w_);
    intVarField_[0].SetFocus();
  }

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

#ifdef ADV
  const bool backendRecordingActive = IsRecordingActive();
  const bool backendSavingActive = IsSavingRecording();

  if (uiRecordingActive_ && !backendRecordingActive) {
    // Backend can stop on its own (duration/RAM limit). Keep the same
    // save-progress and switch flow as a user initiated stop.
    uiRecordingActive_ = false;
    uiSavingActive_ = true;
    autoSwitchPending_ = true;
  }

  if (autoSwitchPending_) {
    // Non-blocking completion check to avoid switching before the record task
    // has finalized and closed the WAV file.
    if (WaitForRecordingStop(0)) {
      FinishStopRecording();
      uiSavingActive_ = false;
      autoSwitchPending_ = false;
      Trace::Log("RECORD", "Recording stopped");

      if (!DidLastRecordingCaptureAudio()) {
        // Treat empty captures as cancelled: stay on record view and resume
        // monitoring instead of switching to sample editor.
        recordingDuration_ = 0;
        updateRecordingSource();
        StartMonitoring();
        isDirty_ = true;
        DrawView();
        return;
      }

      // SampleEditor expects a relative filename for recordings; make sure the
      // working directory points at the recordings folder before switching.
      auto fs = FileSystem::GetInstance();
      if (!fs || !fs->chdir(RECORDINGS_DIR)) {
        Trace::Error("RECORDVIEW: failed to chdir to %s before editor switch",
                     RECORDINGS_DIR);
      }

      // set the current file for sample editor before switching view
      etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> filename(RECORDING_FILENAME);
      viewData_->sampleEditorFilename = filename;

      // Automatically switch to SampleEditor view after recording stops
      ViewType vt = VT_SAMPLE_EDITOR;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      return;
    }
    uiSavingActive_ = true;
  } else {
    uiSavingActive_ = backendSavingActive;
  }
#endif

  if (uiRecordingActive_) {
    // Update recording duration
    uint32_t currentTime = System::GetInstance()->Millis();
    recordingDuration_ = (currentTime - recordingStartTime_);
    if (recordingDuration_ >= kMaxRecordDurationMs) {
      stopAndSwitchToEditor();
      return;
    }
    isDirty_ = true;
    DrawView();
  }
  if (uiSavingActive_) {
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
  if (uiRecordingActive_) {
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
    uiRecordingActive_ = true;
    uiSavingActive_ = false;
    autoSwitchPending_ = false;
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
  const bool backendRecordingActive = IsRecordingActive();
  if (!uiRecordingActive_ && !backendRecordingActive) {
    return;
  }

  RequestStopRecording();
  uiRecordingActive_ = false;
  uiSavingActive_ = true;
  autoSwitchPending_ = true;
  isDirty_ = true;
  DrawView();
#endif
}

void RecordView::stopAndSwitchToEditor() {
#ifdef ADV
  stop();
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

void RecordView::OnFocusLost() {
  Config *config = Config::GetInstance();
  if (!config->Save()) {
    Trace::Error("RECORDVIEW", "Failed to save record setting on focus lost");
    return;
  }
  Trace::Log("RECORDVIEW", "Saved record setting on focus lost");
}
