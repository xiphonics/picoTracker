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

  // Initialize bitmap buffer for waveform display
  // Add our bitmap field, height and width in pixels not character cells!
  // We want a bitmap that is 160 pixels wide and 20 pixels high
  uint16_t bitmapWidth = 200; // Width must be a multiple of 8
  uint16_t bitmapHeight = 40;

  // Allocate 200x40 = 8000 bytes
  uint8_t *buffer = (uint8_t *)malloc((bitmapWidth * bitmapHeight) / 8);

  // Clear the buffer
  bitmapgfx_clear_buffer(buffer, bitmapWidth, bitmapHeight);

  // draw a line lengthwise half way down the bitmap
  bitmapgfx_draw_line(buffer, bitmapWidth, bitmapHeight, 0, bitmapHeight / 2,
                      bitmapWidth - 1, bitmapHeight / 2, true);

  // Draw a rectangle border
  bitmapgfx_draw_rect(buffer, bitmapWidth, bitmapHeight, 0, 0, bitmapWidth,
                      bitmapHeight, false, true);

  GUIPoint position = GetAnchor();

  // Add waveform display field
  position._y = 4;
  position._x = 1;
  waveformField_.emplace_back(position, bitmapWidth, bitmapHeight, buffer,
                              0xFFFF, 0x0000);
  fieldList_.insert(fieldList_.end(), &(*waveformField_.rbegin()));

  // Get the current sample instrument
  // currentInstrument_ = getCurrentSampleInstrument();

  // // Add sample parameters if we have a valid instrument
  // if (currentInstrument_) {
  //   // Add start position control
  //   position._y = 12;
  //   position._x = 5;
  //   Variable *startVar =
  //       currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
  //   if (startVar) {
  //     intVarField_.emplace_back(position, *startVar, "Start: %d", 0, 65535,
  //     1,
  //                               100);
  //     fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  //     (*intVarField_.rbegin()).AddObserver(*this);
  //   }

  //   // Add end position control
  //   position._y += 1;
  //   Variable *endVar =
  //       currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
  //   if (endVar) {
  //     intVarField_.emplace_back(position, *endVar, "End: %d", 0, 65535, 1,
  //     100); fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  //     (*intVarField_.rbegin()).AddObserver(*this);
  //   }

  //   // Add loop start control
  //   position._y += 1;
  //   Variable *loopStartVar =
  //       currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopStart);
  //   if (loopStartVar) {
  //     intVarField_.emplace_back(position, *loopStartVar, "Loop Start: %d", 0,
  //                               65535, 1, 100);
  //     fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  //     (*intVarField_.rbegin()).AddObserver(*this);
  //   }

  //   // Add loop mode control
  //   position._y += 1;
  //   Variable *loopModeVar =
  //       currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopMode);
  //   if (loopModeVar) {
  //     intVarField_.emplace_back(position, *loopModeVar, "Loop Mode: %d", 0,
  //     2,
  //                               1, 1);
  //     fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  //     (*intVarField_.rbegin()).AddObserver(*this);
  //   }
  // }
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
  // if (forceRedraw_) {
  //   updateWaveformDisplay();
  //   forceRedraw_ = false;
  // }
}

void SampleEditorView::AnimationUpdate() {
  // No animation needed for now
}

void SampleEditorView::Update(Observable &o, I_ObservableData *d) {}

void SampleEditorView::updateWaveformDisplay() {
  // TODO

  // if (!currentInstrument_ || !bitmapBuffer_) {
  //   return;
  // }

  // // Clear the bitmap buffer
  // bitmapgfx_clear_buffer(bitmapBuffer_, bitmapWidth_, bitmapHeight_);

  // // Get the sample data
  // SoundSource *source = currentInstrument_->GetSource(0);
  // if (!source) {
  //   return;
  // }

  // // Get sample size
  // int sampleSize = source->GetSize(0);
  // if (sampleSize <= 0) {
  //   // Draw a message if no sample data is available
  //   bitmapgfx_draw_rect(bitmapBuffer_, bitmapWidth_, bitmapHeight_, 0, 0,
  //                       bitmapWidth_ - 1, bitmapHeight_ - 1, false, true);
  //   return;
  // }

  // // Get sample parameters
  // int start = 0;
  // int end = sampleSize - 1;
  // int loopStart = 0;
  // int loopMode = 0;

  // Variable *startVar =
  //     currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
  // if (startVar)
  //   start = startVar->GetInt();

  // Variable *endVar =
  //     currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
  // if (endVar)
  //   end = endVar->GetInt();

  // Variable *loopStartVar =
  //     currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopStart);
  // if (loopStartVar)
  //   loopStart = loopStartVar->GetInt();

  // Variable *loopModeVar =
  //     currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopMode);
  // if (loopModeVar)
  //   loopMode = loopModeVar->GetInt();

  // // Ensure parameters are within valid range
  // if (start >= sampleSize)
  //   start = sampleSize - 1;
  // if (end >= sampleSize)
  //   end = sampleSize - 1;
  // if (loopStart >= sampleSize)
  //   loopStart = sampleSize - 1;

  // // Draw the waveform outline
  // bitmapgfx_draw_rect(bitmapBuffer_, bitmapWidth_, bitmapHeight_, 0, 0,
  //                     bitmapWidth_ - 1, bitmapHeight_ - 1, false, true);

  // // Draw the waveform
  // const short *sampleData = (const short *)source->GetSampleBuffer(0);
  // int centerY = bitmapHeight_ / 2;

  // // Calculate the display range based on start and end positions
  // int displayRange = end - start;
  // if (displayRange <= 0)
  //   displayRange = 1;

  // // Draw the waveform line
  // for (int x = 1; x < bitmapWidth_ - 1; x++) {
  //   int sampleIndex = start + (x * displayRange) / (bitmapWidth_ - 2);
  //   if (sampleIndex >= sampleSize)
  //     break;

  //   // Scale the sample value to fit in the bitmap height
  //   int sampleValue = sampleData[sampleIndex];
  //   int y = centerY - (sampleValue * (centerY - 2)) / 32768;

  //   // Ensure y is within bounds
  //   if (y < 1)
  //     y = 1;
  //   if (y >= bitmapHeight_ - 1)
  //     y = bitmapHeight_ - 2;

  //   // Draw the sample point
  //   bitmapgfx_set_pixel(bitmapBuffer_, bitmapWidth_, x, y, true);
  // }

  // // Draw markers for loop points if loop mode is enabled
  // if (loopMode > 0) {
  //   // Calculate x position for loop start
  //   int loopX = 1 + ((loopStart - start) * (bitmapWidth_ - 2)) /
  //   displayRange; if (loopX >= 1 && loopX < bitmapWidth_ - 1) {
  //     // Draw vertical line for loop start
  //     for (int y = 1; y < bitmapHeight_ - 1; y++) {
  //       bitmapgfx_set_pixel(bitmapBuffer_, bitmapWidth_, loopX, y, true);
  //     }
  //   }
  // }

  // // Update the bitmap field
  // waveformField_[0].SetBitmap(bitmapBuffer_);
}
