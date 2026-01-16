/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_SLICES_VIEW_H_
#define _SAMPLE_SLICES_VIEW_H_

#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "Externals/etl/include/etl/array.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "ViewData.h"
#include <cstdint>

class SampleInstrument;

#ifdef ADV
static constexpr int32_t SliceBitmapWidth = 720;
static constexpr int32_t SliceBitmapHeight = 160;
#else
static constexpr int32_t SliceBitmapWidth = 320;
static constexpr int32_t SliceBitmapHeight = 80;
#endif

static constexpr int32_t SliceWaveformCacheSize = SliceBitmapWidth;
static constexpr size_t SliceCount = 16;

class SliceGraphField : public UIField {
public:
  SliceGraphField(GUIPoint &position, int32_t width, int32_t height);
  ~SliceGraphField() override = default;
  void Draw(GUIWindow &w, int offset = 0) override;
  void OnClick() override{};
  void ProcessArrow(unsigned short) override{};

private:
  int32_t width_;
  int32_t height_;
};

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
  static void AutoSliceConfirmCallback(View &v, ModalView &dialog);
  void buildFieldLayout();
  void rebuildWaveform();
  void drawWaveform();
  SampleInstrument *currentInstrument();
  void updateSliceSelectionFromInstrument();
  void applySliceStart(uint32_t start);
  void autoSliceEvenly();
  void updateZoomLimits();
  bool updateZoomWindow();
  void adjustZoom(int32_t delta);
  void startPreview();
  void stopPreview();
  void handleSliceSelectionChange();
  int32_t sliceToPixel(uint32_t start) const;
  uint32_t selectedSliceStart();
  bool hasInstrumentSample() const;

  WatchedVariable sliceIndexVar_;
  WatchedVariable sliceStartVar_;
  Variable autoSliceCountVar_;

  etl::vector<UIIntVarField, 2> intVarField_;
  etl::vector<UIBigHexVarField, 1> bigHexVarField_;
  etl::vector<UIStaticField, 2> staticField_;
  etl::vector<UIActionField, 1> actionField_;

  uint8_t waveformCache_[SliceWaveformCacheSize];
  bool waveformValid_;
  bool needsWaveformRedraw_;

  SampleInstrument *instrument_;
  int32_t instrumentIndex_;
  uint32_t sampleSize_;
  uint8_t zoomLevel_;
  uint8_t maxZoomLevel_;
  uint32_t viewStart_;
  uint32_t viewEnd_;
  GUIPoint graphFieldPos_;
  SliceGraphField graphField_;
  bool modalWasOpen_;

  bool playKeyHeld_;
  bool previewActive_;
  uint8_t previewNote_;
};

#endif
