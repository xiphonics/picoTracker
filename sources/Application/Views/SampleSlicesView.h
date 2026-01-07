/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_SLICES_VIEW_H_
#define _SAMPLE_SLICES_VIEW_H_

#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "Externals/etl/include/etl/array.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "ViewData.h"

class SampleInstrument;

#ifdef ADV
static constexpr int SliceBitmapWidth = 720;
static constexpr int SliceBitmapHeight = 160;
#else
static constexpr int SliceBitmapWidth = 320;
static constexpr int SliceBitmapHeight = 80;
#endif

static constexpr int SliceWaveformCacheSize = SliceBitmapWidth;
static constexpr size_t SliceCount = 16;

class SampleSlicesView : public FieldView, public I_Observer {
public:
  SampleSlicesView(GUIWindow &w, ViewData *data);
  ~SampleSlicesView() override;

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
  void startPreview();
  void stopPreview();
  void handleSliceSelectionChange();
  int sliceToPixel(uint32_t start) const;
  uint32_t selectedSliceStart() const;
  bool hasInstrumentSample() const;

  WatchedVariable sliceIndexVar_;
  WatchedVariable sliceStartVar_;

  etl::vector<UIIntVarField, 1> intVarField_;
  etl::vector<UIBigHexVarField, 1> bigHexVarField_;
  etl::vector<UIStaticField, 2> staticField_;

  uint8_t waveformCache_[SliceWaveformCacheSize];
  bool waveformValid_;
  bool needsWaveformRedraw_;

  SampleInstrument *instrument_;
  int instrumentIndex_;
  uint32_t sampleSize_;

  bool playKeyHeld_;
  bool previewActive_;
  unsigned char previewNote_;
};

#endif
