/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _GRAPH_FIELD_H_
#define _GRAPH_FIELD_H_

#include "Application/Instruments/SampleInstrument.h"
#include "BaseClasses/UIField.h"
#include "BaseClasses/View.h"
#include <cstdint>

class GraphField : public UIField {
public:
#ifdef ADV
  static constexpr int32_t BitmapWidth = 720;
  static constexpr int32_t BitmapHeight = 180;
#else
  static constexpr int32_t BitmapWidth = 320;
  static constexpr int32_t BitmapHeight = 80;
#endif
  static constexpr int32_t CacheSize = BitmapWidth;
  static constexpr size_t MaxMarkers = SampleInstrument::MaxSlices + 3;

  GraphField(GUIPoint &position, int32_t width, int32_t height);
  ~GraphField() override = default;

  void Draw(GUIWindow &w, int offset = 0) override;
  void OnClick() override{};
  void ProcessArrow(unsigned short) override{};

  void Reset();
  void SetShowBaseline(bool show);
  void SetBorderColors(ColorDefinition normal, ColorDefinition focused);
  void SetSampleSize(uint32_t sampleSize);

  uint32_t SampleSize() const;
  uint8_t ZoomLevel() const;
  uint8_t MaxZoomLevel() const;
  uint32_t ViewStart() const;
  uint32_t ViewEnd() const;
  void ClearWaveformCache();
  bool WaveformValid() const;

  bool UpdateZoomWindow(uint32_t centerSample);
  bool AdjustZoom(int32_t delta, uint32_t centerSample);

  void BeginRmsBuild();
  void AccumulateRmsSample(uint32_t sampleIndex, int16_t sampleValue);
  void FinalizeRmsBuild();

  uint8_t *WaveformCache();
  void InvalidateWaveform();
  void SetWaveformValid(bool valid);
  void RequestFullRedraw();

  void SetMarkerCount(size_t count);
  void SetMarker(size_t index, uint32_t sample, ColorDefinition color,
                 bool visible);

  int32_t SampleToPixel(uint32_t sample) const;
  void DrawGraph(View &view);

private:
  void redrawWaveformColumn(View &view, int32_t x);
  void drawMarkersAt(View &view, int32_t x);
  void resetMarkerCache();
  bool hasValidWindow() const;

  int32_t width_;
  int32_t height_;
  bool showBaseline_;
  ColorDefinition borderNormal_;
  ColorDefinition borderFocused_;

  static uint8_t waveformCache_[CacheSize];
  bool waveformValid_;
  bool needsFullRedraw_;

  uint32_t sampleSize_;
  uint8_t zoomLevel_;
  uint8_t maxZoomLevel_;
  uint32_t viewStart_;
  uint32_t viewEnd_;

  struct Marker {
    uint32_t sample = 0;
    ColorDefinition color = CD_NORMAL;
    bool visible = false;
  };

  Marker markers_[MaxMarkers];
  size_t markerCount_;
  int16_t markerPixelCache_[MaxMarkers];
  ColorDefinition markerColorCache_[MaxMarkers];
  bool markerVisibleCache_[MaxMarkers];

  static int32_t rmsSumSquares_[CacheSize];
  static uint16_t rmsCounts_[CacheSize];
};

#endif
