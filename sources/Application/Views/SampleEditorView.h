
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
#include "Application/Instruments/WavHeader.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "BaseClasses/UITextField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/StringVariable.h"
#include "GraphField.h"
#include "ViewData.h"

class SampleEditorView : public FieldView, public I_Observer {
public:
  SampleEditorView(GUIWindow &w, ViewData *data);
  virtual ~SampleEditorView();
  void Reset();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus();

  // Observer for action callback
  void Update(Observable &, I_ObservableData *);

  void AnimationUpdate() override;

  // Static method to set which view will open the ImportView
  static void SetSourceViewType(ViewType vt);

  // Track which view opened the ImportView (default to project view)
  static ViewType sourceViewType_;

protected:
private:
  static void ConfirmApplyOperationCallback(View &view, ModalView &dialog);
  static void OperationFailedAckCallback(View &view, ModalView &dialog);

  // Helper methods
  void addAllFields();
  void addNameTextField(I_Instrument *instr, GUIPoint &position);
  void updateSampleParameters();
  uint32_t selectionCenterSample() const;
  void updateSelectedMarkerFromFocus();
  void loadSample(const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> path,
                  bool isProjectSampleFile);
  bool reloadEditedSample();
  bool saveSample(etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> &savedFilename);
  bool loadSampleToPool(
      const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> &savedFilename);
  bool applySelectedOperation();
  bool applyTrimOperation(uint32_t startFrame, uint32_t endFrame);
  bool applyNormalizeOperation();
  void navigateToView(ViewType vt);
  SampleInstrument *getCurrentSampleInstrument();
  void clearWaveformRegion();
  void rebuildWaveform();
  void updateGraphMarkers();
  void updateZoomWindow();
  void adjustZoom(int32_t delta);

  // UI fields
  etl::vector<UIIntVarField, 1> intVarField_;
  etl::vector<UIBigHexVarField, 2> bigHexVarField_;
  etl::vector<UIActionField, 4> actionField_;
  etl::vector<UIStaticField, 4> staticField_;
  etl::vector<UITextField<MAX_INSTRUMENT_NAME_LENGTH>, 1> nameTextField_;

  // Flag to force redraw of waveform
  // This is required because we try to not redraw the full waveform as doing so
  // is quite slow
  bool fullWaveformRedraw_;

  // Playback state
  bool isPlaying_;
  bool isSingleCycle_; // Whether the sample is a single cycle waveform
  bool playKeyHeld_;   // Flag to track when the play key is being held down

  // Cached sample parameters
  uint32_t start_ = 0;
  uint32_t end_ = 0;

  void DrawWaveForm();

  float playbackPosition_; // Current playback position as normalized value (0.0
                           // - 1.0)
  uint32_t playbackStartFrame_; // Animation frame when playback started
  uint32_t lastAnimationTime_;  // Timestamp of the last animation frame
  System *sys_;
  uint32_t tempSampleSize_ = 0;
  WavHeaderInfo headerInfo_{};
  bool headerValid_ = false;
  static int16_t chunkBuffer_[512 * 2];
  // Use an empty default name - we don't want to populate with sample
  // filename The display name will still be shown on the phrase screen via
  // GetDisplayName()
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> filename;

  // Variables to back the UI fields
  Variable startVar_;
  Variable endVar_;
  StringVariable<MAX_INSTRUMENT_FILENAME_LENGTH> filenameVar_;
  enum SampleEditOperation { Trim = 0, Normalize };
  Variable operationVar_;

  GUIWindow &win;

  GUIPoint graphFieldPos_;
  GraphField graphField_;

  enum SelectedMarker { MarkerStart = 0, MarkerEnd };
  SelectedMarker selectedMarker_ = MarkerStart;

  uint8_t modalClearCount_ = 0;
};
#endif
