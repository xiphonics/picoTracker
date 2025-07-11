/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_EDITOR_VIEW_H_
#define _SAMPLE_EDITOR_VIEW_H_

#include "Application/Instruments/SampleInstrument.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIBitmapField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "ViewData.h"

class SampleEditorView : public FieldView, public I_Observer {
public:
  SampleEditorView(GUIWindow &w, ViewData *data);
  virtual ~SampleEditorView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus();

  // Observer for action callback
  void Update(Observable &, I_ObservableData *);

  void AnimationUpdate() override;

protected:
private:
  // Helper methods
  SampleInstrument *getCurrentSampleInstrument();
  void updateWaveformDisplay();

  // UI fields
  etl::vector<UIIntVarField, 10> intVarField_;
  etl::vector<UIBigHexVarField, 4> bigHexVarField_;
  etl::vector<UIActionField, 2> actionField_;
  etl::vector<UIStaticField, 4> staticField_;
  etl::vector<UIBitmapField, 1> waveformField_;

#define BITMAPWIDTH 320
#define BITMAPHEIGHT 80
  // Statically allocated bitmap buffer for waveform display
  uint8_t bitmapBuffer_[BITMAPWIDTH * BITMAPHEIGHT / 8];

  // Sample data reference
  SampleInstrument *currentInstrument_;

  // Flag to force redraw of waveform
  bool forceRedraw_;

  // Playback state
  bool isPlaying_;
  bool isSingleCycle_; // Whether the sample is a single cycle waveform

  uint32_t playbackPosition_;
  uint32_t positionScale_;
};
#endif
