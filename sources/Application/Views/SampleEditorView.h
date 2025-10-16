
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
  void addAllFields();
  void addNameTextField(I_Instrument *instr, GUIPoint &position);
  void updateSampleParameters();
  void loadSample(const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> path,
                  bool isProjectSampleFile);

  // UI fields
  etl::vector<UIIntVarField, 1> intVarFields_;
  etl::vector<UIBigHexVarField, 2> bigHexVarFields_;
  etl::vector<UIActionField, 2> actionFields_;
  etl::vector<UIStaticField, 4> staticFields_;
  etl::vector<UITextField<MAX_INSTRUMENT_NAME_LENGTH>, 1> nameTextField_;

#ifdef ADV
#define BITMAPWIDTH 720
#define BITMAPHEIGHT 160
#else
#define BITMAPWIDTH 320
#define BITMAPHEIGHT 80
#endif

// Waveform data cache
#define WAVEFORM_CACHE_SIZE BITMAPWIDTH

  // Flag to force redraw of waveform
  // This is required because we try to not redraw the full waveform as doing so
  // is quite slow
  bool fullWaveformRedraw_;

  // Playback state
  bool isPlaying_;
  bool isSingleCycle_; // Whether the sample is a single cycle waveform
  bool playKeyHeld_;   // Flag to track when the play key is being held down

  // Cached sample parameters
  int start_ = 0;
  int end_ = 0;

  bool goProjectSamplesDir();

  uint8_t waveformCache_[BITMAPWIDTH];
  bool waveformCacheValid_;

  void updateWaveformCache();
  void DrawWaveForm();

  float playbackPosition_; // Current playback position as normalized value (0.0
                           // - 1.0)
  uint32_t playbackStartFrame_; // Animation frame when playback started
  uint32_t lastAnimationTime_;  // Timestamp of the last animation frame
  System *sys_;
  uint32_t tempSampleSize_ = 0;
  static short chunkBuffer_[512 * 2];
  // Use an empty default name - we don't want to populate with sample
  // filename The display name will still be shown on the phrase screen via
  // GetDisplayName()
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> filename;

  // Variables to back the UI fields
  Variable startVar_;
  Variable endVar_;
  Variable filenameVar_;

  GUIWindow &win;

  int last_start_x_ = -1;
  int last_end_x_ = -1;
  int last_playhead_x_ = -1;

  bool redraw_;
};
#endif
