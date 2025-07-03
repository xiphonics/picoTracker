/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SampleEditorView.h"
#include "Adapters/picoTracker/display/chargfx.h"
#include "Application/AppWindow.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Config.h"
#include "Application/Views/BaseClasses/ViewEvent.h"
#include "BaseClasses/UIBitmapField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include <bitmapgfx.h>
#include <cstdint>
#include <nanoprintf.h>

SampleEditorView::SampleEditorView(GUIWindow &w, ViewData *data)
    : FieldView(w, data) {

  // Clear the buffer
  bitmapgfx_clear_buffer(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT);

  GUIPoint position = GetAnchor();

  // Add waveform display field
  position._y = 4;
  position._x = 4;
  waveformField_.emplace_back(position, BITMAPWIDTH, BITMAPHEIGHT,
                              bitmapBuffer_, 0xFFFF, 0x0000);
  fieldList_.insert(fieldList_.end(), &(*waveformField_.rbegin()));

  // Get the current sample instrument
  currentInstrument_ = getCurrentSampleInstrument();

  // Add sample parameters if we have a valid instrument
  if (currentInstrument_) {
    // Add start position control
    position._y = 12;
    position._x = 5;
    Variable *startVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
    if (startVar) {
      intVarField_.emplace_back(position, *startVar, "Start: %d", 0, 65535, 1,
                                100);
      fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
      (*intVarField_.rbegin()).AddObserver(*this);
    }

    // Add end position control
    position._y += 1;
    Variable *endVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
    if (endVar) {
      intVarField_.emplace_back(position, *endVar, "End: %d", 0, 65535, 1, 100);
      fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
      (*intVarField_.rbegin()).AddObserver(*this);
    }

    // Add loop start control
    position._y += 1;
    Variable *loopStartVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopStart);
    if (loopStartVar) {
      intVarField_.emplace_back(position, *loopStartVar, "Loop Start: %d", 0,
                                65535, 1, 100);
      fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
      (*intVarField_.rbegin()).AddObserver(*this);
    }

    // Add loop mode control
    position._y += 1;
    Variable *loopModeVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopMode);
    if (loopModeVar) {
      intVarField_.emplace_back(position, *loopModeVar, "Loop Mode: %d", 0, 2,
                                1, 1);
      fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
      (*intVarField_.rbegin()).AddObserver(*this);
    }
  }
}

SampleEditorView::~SampleEditorView() {}

SampleInstrument *SampleEditorView::getCurrentSampleInstrument() {
  int id = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(id);

  // Check if this is a sample instrument
  if (instr && instr->GetType() == IT_SAMPLE) {
    return static_cast<SampleInstrument *>(instr);
  }

  return nullptr;
}

void SampleEditorView::OnFocus() {
  // Update the current instrument reference
  currentInstrument_ = getCurrentSampleInstrument();

  // Force redraw of waveform
  forceRedraw_ = true;

  // Update the waveform display
  updateWaveformDisplay();
}

void SampleEditorView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed) {
    return;
  }

  FieldView::ProcessButtonMask(mask, pressed);

  if (mask & EPBM_NAV) {
    if (mask & EPBM_LEFT) {
      // Go back to Instrument view with NAV+LEFT
      ViewType vt = VT_INSTRUMENT;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      return;
    }
  } else if (mask & EPBM_PLAY) {
    Player *player = Player::GetInstance();
    player->OnStartButton(PM_SONG, viewData_->songX_, false, viewData_->songX_);
  }
}

void SampleEditorView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title
  char titleString[SCREEN_WIDTH];
  strcpy(titleString, "Sample Edit");

  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, titleString, props);

  FieldView::Redraw();

  SetColor(CD_NORMAL);

  // Update the waveform display if needed
  if (forceRedraw_) {
    updateWaveformDisplay();
    forceRedraw_ = false;
  }
}

void SampleEditorView::AnimationUpdate() {
  // No animation needed for now
}

void SampleEditorView::Update(Observable &o, I_ObservableData *d) {}

void SampleEditorView::updateWaveformDisplay() {
  // if (!currentInstrument_ || !bitmapBuffer_) {
  //   return;
  // }

  // Clear the bitmap buffer
  bitmapgfx_clear_buffer(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT);

  // draw a line lengthwise half way down the bitmap
  bitmapgfx_draw_line(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, 0,
                      BITMAPHEIGHT / 2, BITMAPWIDTH - 1, BITMAPHEIGHT / 2,
                      true);

  // Get sample size directly from the instrument
  int sampleSize = currentInstrument_->GetSampleSize();
  if (sampleSize <= 0) {
    // TODO show some message
    return;
  }

  // Get sample parameters
  int start = 0;
  int end = sampleSize - 1;
  int loopStart = 0;
  int loopMode = 0;

  Variable *startVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
  if (startVar)
    start = startVar->GetInt();

  Variable *endVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
  if (endVar)
    end = endVar->GetInt();

  Variable *loopStartVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopStart);
  if (loopStartVar)
    loopStart = loopStartVar->GetInt();

  Variable *loopModeVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopMode);
  if (loopModeVar)
    loopMode = loopModeVar->GetInt();

  // Ensure parameters are within valid range
  if (start >= sampleSize)
    start = sampleSize - 1;
  if (end >= sampleSize)
    end = sampleSize - 1;
  if (loopStart >= sampleSize)
    loopStart = sampleSize - 1;

  // Draw the waveform outline
  bitmapgfx_draw_rect(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, 0, 0,
                      BITMAPWIDTH - 1, BITMAPHEIGHT - 1, false, true);

  // For now, just draw a placeholder waveform since we can't directly access
  // the sample buffer
  // TODO: Implement proper waveform drawing when we have access to the sample
  // data
  int centerY = BITMAPHEIGHT / 2;

  // Draw a simple sine wave as placeholder
  for (int x = 1; x < BITMAPWIDTH - 1; x++) {
    // Calculate a simple sine wave - use 2π for a complete cycle
    double phase = (double)x / (double)(BITMAPWIDTH - 2) * 2.0 * M_PI;
    // Use a clearer amplitude calculation for better visualization
    int amplitude = (BITMAPHEIGHT / 2) - 2; // Leave 2 pixels margin
    int y = centerY - (int)(amplitude * sin(phase));

    // Ensure y is within bounds
    if (y < 1)
      y = 1;
    if (y >= BITMAPHEIGHT - 1)
      y = BITMAPHEIGHT - 2;

    // y = (x < 50) ? 10 : 30;

    // Draw the sample point
    bitmapgfx_set_pixel(bitmapBuffer_, BITMAPWIDTH, x, y, true);
  }

  // Draw markers for loop points if loop mode is enabled
  if (loopMode > 0) {
    // Calculate x position for loop start
    int totalRange = end - start;
    if (totalRange <= 0)
      totalRange = 1;

    int loopX = 1 + ((loopStart - start) * (BITMAPWIDTH - 2)) / totalRange;
    if (loopX >= 1 && loopX < BITMAPWIDTH - 1) {
      // Draw vertical line for loop start
      for (int y = 1; y < BITMAPHEIGHT - 1; y++) {
        bitmapgfx_set_pixel(bitmapBuffer_, BITMAPWIDTH, loopX, y, true);
      }
    }
  }

  // Update the bitmap field
  waveformField_[0].SetBitmap(bitmapBuffer_);
}
