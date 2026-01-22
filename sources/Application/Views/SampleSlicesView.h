/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_SLICES_VIEW_H_
#define _SAMPLE_SLICES_VIEW_H_

#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "GraphField.h"
#include "System/System/System.h"
#include "ViewData.h"
#include <cstdint>

class SampleInstrument;

class SampleSlicesView : public FieldView, public I_Observer {
public:
  SampleSlicesView(GUIWindow &w, ViewData *data);
  ~SampleSlicesView() override;
  void Reset();

  void ProcessButtonMask(unsigned short mask, bool pressed) override;
  void DrawView() override;
  void OnPlayerUpdate(PlayerEventType, unsigned int) override{};
  void OnFocus() override;
  void AnimationUpdate() override;
  void Update(Observable &o, I_ObservableData *d) override;

private:
  void buildFieldLayout();
  void rebuildWaveform();
  void drawWaveform();
  SampleInstrument *currentInstrument();
  void updateSliceSelectionFromInstrument();
  void applySliceStart(uint32_t start);
  void updateZoomLimits();
  bool updateZoomWindow();
  void adjustZoom(int32_t delta);
  void startPreview();
  void stopPreview();
  uint32_t sliceEndForIndex(size_t index, uint32_t start) const;
  void handleSliceSelectionChange();
  uint32_t selectedSliceStart();
  bool hasInstrumentSample() const;

  WatchedVariable sliceIndexVar_;
  WatchedVariable sliceStartVar_;

  etl::vector<UIIntVarField, 1> intVarField_;
  etl::vector<UIStaticField, 5> staticField_;

  bool needsFullRedraw_;

  SampleInstrument *instrument_;
  int32_t instrumentIndex_;
  uint32_t sampleSize_;
  GUIPoint graphFieldPos_;
  GraphField graphField_;
  bool modalWasOpen_;

  bool playKeyHeld_;
  bool previewActive_;
  uint8_t previewNote_;
  System *sys_;
  uint32_t previewStartMs_;
  uint32_t previewStartSample_;
  uint32_t previewEndSample_;
  float previewDurationMs_;
  uint32_t previewPlayheadSample_;
  bool previewCursorVisible_;
};

#endif
