/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "RecordView.h"
#include "Application/Model/Config.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "UIController.h"
#include "ViewData.h"

#ifdef PLATFORM_ADV
#include "Adapters/adv/audio/record.h"
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

  // Audio source selection field (Line In = 0, Mic = 1)
  Variable *v = config->FindVariable(FourCC::VarRecordSource);
  intVarField_.emplace_back(position, *v, "Audio source: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

  position._y += 2;
}

RecordView::~RecordView() {
  if (isRecording_) {
    stopRecording();
  }
}

void RecordView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed) {
    return;
  }

  // TODO: temp hack to always nav back to song screen
  if (mask & EPBM_NAV) {
    if (mask & EPBM_LEFT) {
      ViewType vt = VT_SONG;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      return;
    }
  }

  // Handle PLAY button for start/stop recording
  if (mask & EPBM_PLAY) {
    if (isRecording_) {
      stopRecording();
    } else {
      startRecording();
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

  // Draw fields
  FieldView::DrawView();

  // Draw recording status and time
  pos = GetAnchor();
  pos._y += 3;

  SetColor(CD_NORMAL);

  if (isRecording_) {
    SetColor(CD_HILITE1);
    DrawString(pos._x, pos._y, "REC", props);
    SetColor(CD_NORMAL);
  } else {
    DrawString(pos._x, pos._y, "---", props);
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
  DrawString(pos._x, pos._y, "PLAY: Start/Stop", props);

  SetColor(CD_NORMAL);
}

void RecordView::OnFocus() {
  // Reset recording state when view gains focus
  if (isRecording_) {
    stopRecording();
  }
  isDirty_ = true;
}

void RecordView::Update(Observable &o, I_ObservableData *d) {
  // Handle field updates
  isDirty_ = true;
  Config::GetInstance()->Save();
}

void RecordView::AnimationUpdate() {
  if (isRecording_) {
    // Update recording duration
    uint32_t currentTime = System::GetInstance()->GetClock();
    recordingDuration_ = currentTime - recordingStartTime_;
    isDirty_ = true;
  }

  // Draw battery and power button UI
  GUITextProperties props;
  drawBattery(props);
  drawPowerButtonUI(props);
}

void RecordView::startRecording() {
#ifdef PLATFORM_ADV
  if (isRecording_) {
    return;
  }

  // Generate filename
  char filename[64];
  generateFilename(filename, sizeof(filename));

  // Get audio source setting (0 = Line In, 1 = Mic)
  int audioSource = intVarField_[0].GetInt();

  Trace::Log("RECORD", "Starting recording to %s, source: %s", filename,
             audioSource == 0 ? "Line In" : "Mic");

  // Start recording with threshold and duration
  // Using 20 second duration for now (20000 ms)
  bool success = StartRecording(filename, 10, 20000);

  if (success) {
    isRecording_ = true;
    recordingStartTime_ = System::GetInstance()->GetClock();
    recordingDuration_ = 0;
    Trace::Log("RECORD", "Recording started successfully");
  } else {
    Trace::Error("RECORD", "Failed to start recording");
  }
#else
  Trace::Log("RECORD", "Recording not supported on this platform");
#endif
}

void RecordView::stopRecording() {
#ifdef PLATFORM_ADV
  if (!isRecording_) {
    return;
  }

  stopRecording();
  isRecording_ = false;

  Trace::Log("RECORD", "Recording stopped");
#endif
}

void RecordView::generateFilename(char *buffer, size_t bufferSize) {
  // Generate auto filename like REC001.wav, REC002.wav, etc.
  static int recordCounter = 1;
  snprintf(buffer, bufferSize, "REC%03d.wav", recordCounter++);
}

void RecordView::formatTime(uint32_t milliseconds, char *buffer,
                            size_t bufferSize) {
  uint32_t seconds = milliseconds / 1000;
  uint32_t minutes = seconds / 60;
  seconds = seconds % 60;

  snprintf(buffer, bufferSize, "%02d:%02d", (int)minutes, (int)seconds);
}
