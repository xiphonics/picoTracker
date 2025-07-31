
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

#include "Application/Instruments/InstrumentNameVariable.h"
#include "Application/Instruments/SampleInstrument.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIBitmapField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "BaseClasses/UITextField.h"
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
  void addAllFields();
  void addNameTextField(I_Instrument *instr, GUIPoint &position);

  // UI fields
  etl::vector<UIIntVarField, 10> intVarField_;
  etl::vector<UIBigHexVarField, 4> bigHexVarField_;
  etl::vector<UIActionField, 2> actionField_;
  etl::vector<UIStaticField, 4> staticField_;
  etl::vector<UIBitmapField, 1> waveformField_;
  etl::vector<UITextField<MAX_INSTRUMENT_NAME_LENGTH>, 1> nameTextField_;
  etl::vector<InstrumentNameVariable, 1> nameVariables_;

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
  bool playKeyHeld_;   // Flag to track when the play key is being held down

private:
  // Waveform data cache
  static const int WAVEFORM_CACHE_SIZE = 320; // Match BITMAPWIDTH
  uint8_t
      waveformCache_[WAVEFORM_CACHE_SIZE]; // Store pre-calculated pixel heights
  bool waveformCacheValid_;

  void updateWaveformCache();

  float playbackPosition_; // Current playback position as normalized value (0.0
                           // - 1.0)
  uint32_t playbackStartFrame_; // Animation frame when playback started
  uint32_t lastAnimationTime_;  // Timestamp of the last animation frame
  System *sys_;
};
#endif
