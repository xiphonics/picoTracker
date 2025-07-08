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
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIBitmapField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include <bitmapgfx.h>
#include <cstdint>
#include <nanoprintf.h>

SampleEditorView::SampleEditorView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), forceRedraw_(true) {

  // Clear the buffer
  bitmapgfx_clear_buffer(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT);

  GUIPoint position = GetAnchor();

  // Add waveform display field
  position._y = 4;
  position._x = 5;
  waveformField_.emplace_back(position, BITMAPWIDTH, BITMAPHEIGHT,
                              bitmapBuffer_, 0xFFFF, 0x0000);
  fieldList_.insert(fieldList_.end(), &(*waveformField_.rbegin()));

  // Get the current sample instrument
  currentInstrument_ = getCurrentSampleInstrument();

  // Add sample parameters if we have a valid instrument
  if (currentInstrument_) {
    int sampleSize = currentInstrument_->GetSampleSize();

    // Add start position control
    position._y = 10;
    position._x = 5;
    Variable *startVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
    if (startVar) {
      bigHexVarField_.emplace_back(position, *startVar, 7, "start: %7.7X", 0,
                                   sampleSize - 1, 16);
      fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
      (*bigHexVarField_.rbegin()).AddObserver(*this);
    }

    // Add loop start control
    position._y += 1;
    Variable *loopStartVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopStart);
    if (loopStartVar) {
      bigHexVarField_.emplace_back(position, *loopStartVar, 7,
                                   "loop start: %7.7X", 0, sampleSize - 1, 16);
      fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
      (*bigHexVarField_.rbegin()).AddObserver(*this);
    }

    // Add end position control
    position._y += 1;
    Variable *endVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
    if (endVar) {
      bigHexVarField_.emplace_back(position, *endVar, 7, "loop end: %7.7X", 0,
                                   sampleSize - 1, 16);
      fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
      (*bigHexVarField_.rbegin()).AddObserver(*this);
    }

    // Add loop mode control
    position._y += 1;
    Variable *loopModeVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopMode);
    if (loopModeVar) {
      intVarField_.emplace_back(position, *loopModeVar, "loop mode: %s", 0, 2,
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
  printf("DEBUG: DrawView called\n");
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

  // DrawView should not directly update the waveform
  // That's now handled in AnimationUpdate
  printf("DEBUG: DrawView completed (forceRedraw_=%d)\n", forceRedraw_);
}

void SampleEditorView::AnimationUpdate() {
  // Check if we need to update the waveform display
  if (forceRedraw_) {
    printf("DEBUG: AnimationUpdate - updating waveform display\n");
    updateWaveformDisplay();
    forceRedraw_ = false;
  }

  GUITextProperties props;
  drawBattery(props);
  drawPowerButtonUI(props);
}

void SampleEditorView::Update(Observable &o, I_ObservableData *d) {
  // When any of our observed variables change, force a redraw of the waveform
  forceRedraw_ = true;

  // Simple debug logging
  printf("DEBUG: Update called, setting forceRedraw_ flag\n");
}

void SampleEditorView::updateWaveformDisplay() {
  // Clear the bitmap buffer
  memset(bitmapBuffer_, 0, BITMAPWIDTH * BITMAPHEIGHT / 8);

  // Draw a border around the waveform display
  bitmapgfx_draw_rect(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, 0, 0,
                      BITMAPWIDTH - 1, BITMAPHEIGHT - 1, false, true);

  // Check if we have a valid instrument
  if (!currentInstrument_) {
    // No instrument, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
    return;
  }

  // Get sample size directly from the instrument
  int sampleSize = currentInstrument_->GetSampleSize();
  if (sampleSize <= 0) {
    // No sample data, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
    Trace::Debug("No sample data, just update the bitmap");
    return;
  }

  // Get the sample data from the instrument
  int sampleIndex = currentInstrument_->GetSampleIndex();
  if (sampleIndex < 0) {
    // No valid sample index, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
    return;
  }

  // Get the sample source from the sample pool
  SamplePool *pool = SamplePool::GetInstance();
  SoundSource *source = pool->GetSource(sampleIndex);
  if (!source) {
    // No valid source, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
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
  if (start >= sampleSize) {
    start = sampleSize - 1;
  }
  if (end >= sampleSize) {
    end = sampleSize - 1;
  }
  if (loopStart >= sampleSize) {
    loopStart = sampleSize - 1;
  }

  // Get the sample buffer
  void *sampleBufferVoid = source->GetSampleBuffer(0); // Use note 0 as default
  if (!sampleBufferVoid) {
    // No sample buffer, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
    return;
  }

  // For now just assume 16-bit samples
  short *sampleBuffer = (short *)sampleBufferVoid;

  // Calculate how many samples to skip between each pixel
  // Use the full sample size to maintain scale
  float samplesPerPixel = (float)sampleSize / (BITMAPWIDTH - 2);

  // Draw the actual waveform from the sample data
  int centerY = BITMAPHEIGHT / 2;

  for (int x = 1; x < BITMAPWIDTH - 1; x++) {
    // Calculate the sample index for this x position
    int sampleIndex = (int)((x - 1) * samplesPerPixel);

    // Ensure the sample index is within bounds
    if (sampleIndex < 0)
      sampleIndex = 0;
    if (sampleIndex >= sampleSize)
      sampleIndex = sampleSize - 1;

    // Get the sample value and map it to the bitmap height
    short sampleValue = sampleBuffer[sampleIndex];

    // Map the 16-bit sample (-32768 to 32767) to the bitmap height
    int y = centerY - (int)((sampleValue * (BITMAPHEIGHT - 4)) / 65536);

    // Ensure y is within bounds
    if (y < 1)
      y = 1;
    if (y >= BITMAPHEIGHT - 1)
      y = BITMAPHEIGHT - 2;

    // Draw the sample point
    bitmapgfx_set_pixel(bitmapBuffer_, BITMAPWIDTH, x, y, true);
  }

  // Get the full sample range for proper scaling
  int fullSampleSize = currentInstrument_->GetSampleSize();

  // Calculate positions for start and end markers based on their actual values
  // Map the start position to the bitmap width - start as a fraction of
  // fullSampleSize
  int startX = 1 + (int)(((float)start / fullSampleSize) * (BITMAPWIDTH - 2));
  if (startX < 1)
    startX = 1;
  if (startX >= BITMAPWIDTH - 1)
    startX = BITMAPWIDTH - 2;

  printf(
      "DEBUG: Drawing start point line at x=%d (start=%d, fullSampleSize=%d)\n",
      startX, start, fullSampleSize);

  // Draw the start marker line
  bitmapgfx_draw_line(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, startX, 1,
                      startX, BITMAPHEIGHT - 2, true);

  // Map the end position to the bitmap width - end as a fraction of
  // fullSampleSize
  int endX = 1 + (int)(((float)end / fullSampleSize) * (BITMAPWIDTH - 2));
  if (endX < 1)
    endX = 1;
  if (endX >= BITMAPWIDTH - 1)
    endX = BITMAPWIDTH - 2;

  printf("DEBUG: Drawing end point line at x=%d (end=%d, fullSampleSize=%d)\n",
         endX, end, fullSampleSize);

  // Draw the end marker line
  bitmapgfx_draw_line(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, endX, 1, endX,
                      BITMAPHEIGHT - 2, true);

  // Draw markers for loop points if loop mode is enabled
  // draw loop start marker as dashed line
  if (loopMode > 0) {
    // Calculate x position for loop start using the same scaling as start/end
    // markers - loopStart as a fraction of fullSampleSize
    int loopX =
        1 + (int)(((float)loopStart / fullSampleSize) * (BITMAPWIDTH - 2));
    if (loopX >= 1 && loopX < BITMAPWIDTH - 1) {
      // Draw a dashed vertical line for loop start
      for (int y = 1; y < BITMAPHEIGHT - 2; y += 3) {
        // Draw 2-pixel dash, then 1-pixel gap
        bitmapgfx_set_pixel(bitmapBuffer_, BITMAPWIDTH, loopX, y, true);
        bitmapgfx_set_pixel(bitmapBuffer_, BITMAPWIDTH, loopX, y + 1, true);
      }
      printf("DEBUG: Drawing loop start line at x=%d (loopStart=%d, "
             "fullSampleSize=%d)\n",
             loopX, loopStart, fullSampleSize);
    }
  }

  // Update the bitmap field
  if (waveformField_.size() > 0) {
    printf("DEBUG: Updating bitmap field with new bitmap data\n");
    waveformField_[0].SetBitmap(bitmapBuffer_);
    printf("DEBUG: Setting view dirty flag to force redraw\n");
    SetDirty(true);
  }
}
