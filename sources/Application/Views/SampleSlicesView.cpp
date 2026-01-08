/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SampleSlicesView.h"

#include "Application/AppWindow.h"
#include "Application/Instruments/InstrumentBank.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Project.h"
#include "Application/Model/Song.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "System/Console/Trace.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
constexpr uint16_t PreviewChannel = SONG_CHANNEL_COUNT - 1;
constexpr int32_t SliceXOffset = 0;
#ifdef ADV
constexpr int32_t SliceYOffset = 2 * CHAR_HEIGHT * 4;
#else
constexpr int32_t SliceYOffset = 2 * CHAR_HEIGHT;
#endif
} // namespace

SliceGraphField::SliceGraphField(GUIPoint &position, int32_t width,
                                 int32_t height)
    : UIField(position), width_(width), height_(height) {}

void SliceGraphField::Draw(GUIWindow &w, int offset) {
  if (!focus_) {
    return;
  }
  int32_t x = x_;
  int32_t y = static_cast<int32_t>(y_) + offset;
  int32_t right = x + width_;
  int32_t bottom = y + height_;

  w.SetCurrentRectColor(AppWindow::GetColor(CD_HILITE2));
  GUIRect top(x, y, right, y + 1);
  GUIRect bottomLine(x, bottom - 1, right, bottom);
  GUIRect left(x, y, x + 1, bottom);
  GUIRect rightLine(right - 1, y, right, bottom);
  w.DrawRect(top);
  w.DrawRect(bottomLine);
  w.DrawRect(left);
  w.DrawRect(rightLine);
  w.SetCurrentRectColor(AppWindow::GetColor(CD_NORMAL));
}

SampleSlicesView::SampleSlicesView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), sliceIndexVar_(FourCC::SampleInstrumentSlices, 0),
      sliceStartVar_(FourCC::SampleInstrumentStart, 0), waveformValid_(false),
      needsWaveformRedraw_(true), instrument_(nullptr), instrumentIndex_(0),
      sampleSize_(0), zoomLevel_(0), maxZoomLevel_(0), viewStart_(0),
      viewEnd_(0), graphFieldPos_(SliceXOffset, SliceYOffset),
      graphField_(graphFieldPos_, SliceBitmapWidth, SliceBitmapHeight),
      playKeyHeld_(false), previewActive_(false),
      previewNote_(SampleInstrument::SliceNoteBase) {
  sliceIndexVar_.AddObserver(*this);
  sliceStartVar_.AddObserver(*this);
  std::memset(waveformCache_, 0, sizeof(waveformCache_));
}

SampleSlicesView::~SampleSlicesView() { stopPreview(); }

void SampleSlicesView::OnFocus() {
  stopPreview();
  instrumentIndex_ = static_cast<int32_t>(viewData_->currentInstrumentID_);
  instrument_ = currentInstrument();
  playKeyHeld_ = false;
  previewActive_ = false;
  waveformValid_ = false;
  needsWaveformRedraw_ = true;
  sampleSize_ = 0;
  zoomLevel_ = 0;

  if (instrument_) {
    SamplePool *pool = SamplePool::GetInstance();
    int32_t sampleIndex = instrument_->GetSampleIndex();
    if (sampleIndex >= 0) {
      if (SoundSource *source = pool->GetSource(sampleIndex)) {
        int32_t size = source->GetSize(0);
        sampleSize_ = (size > 0) ? static_cast<uint32_t>(size) : 0;
      }
    }
  }

  sliceIndexVar_.SetInt(0, false);
  updateSliceSelectionFromInstrument();
  updateZoomLimits();
  updateZoomWindow();
  rebuildWaveform();
  buildFieldLayout();
  isDirty_ = true;
}

void SampleSlicesView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed) {
    if (playKeyHeld_ && !(mask & EPBM_PLAY)) {
      playKeyHeld_ = false;
      stopPreview();
      needsWaveformRedraw_ = true;
    }
    FieldView::ProcessButtonMask(mask, pressed);
    return;
  }

  if (mask & EPBM_NAV) {
    if (mask & EPBM_LEFT) {
      // Go back to sample browser NAV+LEFT
      stopPreview();
      ViewType vt = VT_INSTRUMENT;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      return;
    }
    // For other NAV combinations, let parent handle it
    FieldView::ProcessButtonMask(mask, pressed);
    return;
  }

  if (mask & EPBM_PLAY) {
    if (!playKeyHeld_) {
      startPreview();
      playKeyHeld_ = true;
      needsWaveformRedraw_ = true;
    }
    return;
  }

  bool graphFocused = (GetFocus() == &graphField_);

  if (graphFocused && (mask & EPBM_EDIT)) {
    if (mask & EPBM_LEFT) {
      int32_t index = sliceIndexVar_.GetInt();
      if (index > 0) {
        sliceIndexVar_.SetInt(index - 1);
      }
      return;
    }
    if (mask & EPBM_RIGHT) {
      int32_t index = sliceIndexVar_.GetInt();
      if (index < static_cast<int32_t>(SliceCount) - 1) {
        sliceIndexVar_.SetInt(index + 1);
      }
      return;
    }
  }

  if (graphFocused && (mask & EPBM_ENTER)) {
    uint32_t viewSpan = (viewEnd_ > viewStart_) ? (viewEnd_ - viewStart_) : 0;
    if (viewSpan == 0) {
      updateZoomWindow();
      viewSpan = (viewEnd_ > viewStart_) ? (viewEnd_ - viewStart_) : 0;
    }
    if (viewSpan > 0) {
      int32_t delta = 0;
      if (mask & (EPBM_LEFT | EPBM_RIGHT)) {
        delta = static_cast<int32_t>(std::max<uint32_t>(1, viewSpan / 64));
        if (mask & EPBM_LEFT) {
          delta = -delta;
        }
      } else if (mask & (EPBM_UP | EPBM_DOWN)) {
        delta = static_cast<int32_t>(std::max<uint32_t>(1, viewSpan / 16));
        if (mask & EPBM_DOWN) {
          delta = -delta;
        }
      }
      if (delta != 0) {
        int32_t start = sliceStartVar_.GetInt();
        int32_t newStart = start + delta;
        if (newStart < 0) {
          newStart = 0;
        }
        if (sampleSize_ > 0) {
          int32_t maxStart = static_cast<int32_t>(sampleSize_ - 1);
          if (newStart > maxStart) {
            newStart = maxStart;
          }
        }
        sliceStartVar_.SetInt(newStart);
        return;
      }
    }
  }
  // We allow zooming from any place of the screen
  if ((mask & EPBM_EDIT) && (mask & EPBM_UP)) {
    adjustZoom(1);
    return;
  }
  if ((mask & EPBM_EDIT) && (mask & EPBM_DOWN)) {
    adjustZoom(-1);
    return;
  }

  FieldView::ProcessButtonMask(mask, pressed);
}

void SampleSlicesView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint titlePos = GetTitlePosition();
  DrawString(titlePos._x, titlePos._y, "Sample Slices", props);

  drawWaveform();
  needsWaveformRedraw_ = false;

  FieldView::Redraw();
}

void SampleSlicesView::AnimationUpdate() {
  GUITextProperties props;
  drawBattery(props);
  drawPowerButtonUI(props);
}

void SampleSlicesView::Update(Observable &o, I_ObservableData *d) {
  if (!hasFocus_) {
    return;
  }

  uintptr_t fourcc = (uintptr_t)d;

  switch (fourcc) {
  case FourCC::SampleInstrumentSlices:
    handleSliceSelectionChange();
    ((AppWindow &)w_).SetDirty();
    break;
  case FourCC::SampleInstrumentStart:
    applySliceStart(static_cast<uint32_t>(sliceStartVar_.GetInt()));
    needsWaveformRedraw_ = true;
    isDirty_ = true;
    ((AppWindow &)w_).SetDirty();
    break;
  default:
    break;
  }
}

void SampleSlicesView::buildFieldLayout() {
  for (auto &f : intVarField_) {
    f.RemoveObserver(*this);
  }
  for (auto &f : bigHexVarField_) {
    f.RemoveObserver(*this);
  }

  fieldList_.clear();
  intVarField_.clear();
  bigHexVarField_.clear();
  staticField_.clear();

  GUIPoint position = GetAnchor();
  position._x += 5;
  position._y = 12;

  intVarField_.emplace_back(position, sliceIndexVar_, "slice: %d", 0,
                            static_cast<int32_t>(SliceCount) - 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &intVarField_.back());
  intVarField_.back().AddObserver(*this);

  position._y += 1;
  int32_t maxStart =
      (sampleSize_ > 0) ? static_cast<int32_t>(sampleSize_ - 1) : 0;
  int32_t minStart = 0;
  if (instrument_) {
    int32_t index = sliceIndexVar_.GetInt();
    if (index > 0) {
      minStart = static_cast<int32_t>(instrument_->GetSlicePoint(index - 1));
    }
  }
  bigHexVarField_.emplace_back(position, sliceStartVar_, 7, "position: %7.7X",
                               minStart, maxStart, 16);
  fieldList_.insert(fieldList_.end(), &bigHexVarField_.back());
  bigHexVarField_.back().AddObserver(*this);

  fieldList_.insert(fieldList_.end(), &graphField_);

  position._y += 2;
  position._x = GetAnchor()._x + 5;
  staticField_.emplace_back(position, "PLAY: preview slice");
  fieldList_.insert(fieldList_.end(), &staticField_.back());

  if (!fieldList_.empty()) {
    SetFocus(*fieldList_.begin());
  }
}

void SampleSlicesView::rebuildWaveform() {
  waveformValid_ = false;
  std::memset(waveformCache_, 0, sizeof(waveformCache_));

  if (!instrument_) {
    return;
  }

  SamplePool *pool = SamplePool::GetInstance();
  int32_t sampleIndex = instrument_->GetSampleIndex();
  if (sampleIndex < 0) {
    return;
  }

  SoundSource *source = pool->GetSource(sampleIndex);
  if (!source) {
    return;
  }

  int32_t size = source->GetSize(0);
  sampleSize_ = (size > 0) ? static_cast<uint32_t>(size) : 0;
  if (sampleSize_ == 0) {
    return;
  }
  if (viewEnd_ <= viewStart_) {
    return;
  }

  int32_t channels = source->GetChannelCount(0);
  int16_t *samples = static_cast<int16_t *>(source->GetSampleBuffer(0));
  if (!samples) {
    return;
  }

  std::fill(std::begin(waveformCache_), std::end(waveformCache_), 0);
  // We quantize to 8-bit in order to accumulate into int32_t to save memory
  static int32_t sumSquares[SliceWaveformCacheSize];
  // Counts fit in uint16_t: max bucket size is <= total samples / columns.
  static uint16_t counts[SliceWaveformCacheSize];
  std::fill_n(sumSquares, SliceWaveformCacheSize, int32_t{0});
  std::fill_n(counts, SliceWaveformCacheSize, uint16_t{0});
  uint32_t viewSpan = viewEnd_ - viewStart_;
  if (viewSpan == 0) {
    return;
  }

  for (uint32_t i = viewStart_; i < viewEnd_; ++i) {
    uint32_t rel = i - viewStart_;
    uint32_t pixel = static_cast<uint32_t>(
        (static_cast<uint64_t>(rel) * SliceWaveformCacheSize) / viewSpan);
    if (pixel >= SliceWaveformCacheSize) {
      pixel = SliceWaveformCacheSize - 1;
    }
    int16_t value = samples[i * channels];
    int16_t quant = static_cast<int16_t>(value >> 8);
    sumSquares[pixel] += static_cast<int32_t>(quant) * quant;
    counts[pixel]++;
  }

  for (int32_t i = 0; i < SliceWaveformCacheSize; ++i) {
    if (counts[i] == 0) {
      waveformCache_[i] = 0;
      continue;
    }
    float meanSquare = static_cast<float>(sumSquares[i]) / counts[i];
    float rms = std::sqrt(meanSquare) / 128.0f;
    uint8_t height = static_cast<uint8_t>(std::min<float>(
        rms * SliceBitmapHeight, static_cast<float>(SliceBitmapHeight)));
    waveformCache_[i] = height;
  }

  waveformValid_ = true;
  needsWaveformRedraw_ = true;
}

void SampleSlicesView::drawWaveform() {
  GUIRect area(SliceXOffset, SliceYOffset, SliceXOffset + SliceBitmapWidth,
               SliceYOffset + SliceBitmapHeight);
  DrawRect(area, CD_BACKGROUND);

  if (!waveformValid_) {
    rebuildWaveform();
  }

  if (waveformValid_) {
    int32_t centerY = SliceYOffset + SliceBitmapHeight / 2;
    for (int32_t x = 1; x < SliceBitmapWidth - 1; ++x) {
      uint8_t amplitude = waveformCache_[x - 1];
      if (amplitude == 0) {
        continue;
      }
      int32_t startY = centerY - amplitude / 2;
      int32_t endY = startY + amplitude;
      GUIRect column(SliceXOffset + x, startY, SliceXOffset + x + 1, endY);
      DrawRect(column, CD_NORMAL);
    }
  }

  if (!instrument_ || sampleSize_ == 0) {
    return;
  }

  for (size_t i = 0; i < SliceCount; ++i) {
    if (!instrument_->IsSliceDefined(i)) {
      continue;
    }
    uint32_t start = instrument_->GetSlicePoint(i);
    if (i == 0 && start == 0 && !instrument_->HasSlicesForPlayback()) {
      continue;
    }
    int32_t x = sliceToPixel(start);
    if (x < 0) {
      continue;
    }
    ColorDefinition color = (static_cast<int32_t>(i) == sliceIndexVar_.GetInt())
                                ? CD_HILITE2
                                : CD_ACCENT;
    GUIRect marker(x, SliceYOffset + 2, x + 1,
                   SliceYOffset + SliceBitmapHeight - 2);
    DrawRect(marker, color);
  }
}

SampleInstrument *SampleSlicesView::currentInstrument() {
  if (!viewData_ || !viewData_->project_) {
    return nullptr;
  }

  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  if (!bank) {
    return nullptr;
  }

  I_Instrument *instr = bank->GetInstrument(viewData_->currentInstrumentID_);
  if (!instr || instr->GetType() != IT_SAMPLE) {
    return nullptr;
  }
  return static_cast<SampleInstrument *>(instr);
}

void SampleSlicesView::updateSliceSelectionFromInstrument() {
  if (!instrument_) {
    sliceStartVar_.SetInt(0, false);
    return;
  }

  int32_t index = sliceIndexVar_.GetInt();
  if (index < 0) {
    index = 0;
  }
  if (index >= static_cast<int32_t>(SliceCount)) {
    index = static_cast<int32_t>(SliceCount) - 1;
  }

  size_t sliceIndex = static_cast<size_t>(index);
  uint32_t start = instrument_->GetSlicePoint(sliceIndex);
  if (start == 0 && index > 0) {
    for (int32_t i = index - 1; i >= 0; --i) {
      uint32_t prev = instrument_->GetSlicePoint(static_cast<size_t>(i));
      if (prev > 0) {
        start = prev;
        break;
      }
      if (i == 0 && instrument_->HasSlicesForPlayback()) {
        start = 0;
        break;
      }
    }
  }
  sliceStartVar_.SetInt(static_cast<int32_t>(start), false);
}

void SampleSlicesView::applySliceStart(uint32_t start) {
  if (!instrument_) {
    return;
  }
  size_t index = static_cast<size_t>(sliceIndexVar_.GetInt());
  instrument_->SetSlicePoint(index, start);
  uint32_t stored = instrument_->GetSlicePoint(index);
  if (stored != start) {
    sliceStartVar_.SetInt(static_cast<int32_t>(stored), false);
  }
  if (updateZoomWindow()) {
    waveformValid_ = false;
    needsWaveformRedraw_ = true;
    isDirty_ = true;
  }
}

void SampleSlicesView::updateZoomLimits() {
  maxZoomLevel_ = 0;
  uint32_t span = sampleSize_;
  while (span > static_cast<uint32_t>(SliceWaveformCacheSize) &&
         maxZoomLevel_ < 16) {
    span = (span + 1) / 2;
    maxZoomLevel_++;
  }
  if (zoomLevel_ > maxZoomLevel_) {
    zoomLevel_ = maxZoomLevel_;
  }
}

bool SampleSlicesView::updateZoomWindow() {
  if (sampleSize_ == 0) {
    viewStart_ = 0;
    viewEnd_ = 0;
    return false;
  }

  uint32_t zoomFactor =
      (zoomLevel_ < 31) ? (static_cast<uint32_t>(1) << zoomLevel_) : 0;
  if (zoomFactor == 0) {
    zoomFactor = 1;
  }
  uint32_t viewSpan = sampleSize_ / zoomFactor;
  if (viewSpan == 0) {
    viewSpan = 1;
  }
  if (viewSpan >= sampleSize_) {
    viewSpan = sampleSize_;
  }

  uint32_t center = selectedSliceStart();
  if (center >= sampleSize_) {
    center = sampleSize_ - 1;
  }

  uint32_t start = 0;
  if (viewSpan < sampleSize_) {
    uint32_t half = viewSpan / 2;
    if (center > half) {
      start = center - half;
    }
    if (start + viewSpan > sampleSize_) {
      start = sampleSize_ - viewSpan;
    }
  }
  uint32_t end = start + viewSpan;

  bool changed = (start != viewStart_) || (end != viewEnd_);
  viewStart_ = start;
  viewEnd_ = end;
  return changed;
}

void SampleSlicesView::adjustZoom(int32_t delta) {
  if (sampleSize_ == 0) {
    return;
  }
  int32_t newLevel =
      static_cast<int32_t>(zoomLevel_) + static_cast<int32_t>(delta);
  if (newLevel < 0) {
    newLevel = 0;
  }
  if (newLevel > static_cast<int32_t>(maxZoomLevel_)) {
    newLevel = static_cast<int32_t>(maxZoomLevel_);
  }
  if (newLevel == static_cast<int32_t>(zoomLevel_)) {
    return;
  }
  zoomLevel_ = static_cast<uint8_t>(newLevel);
  if (updateZoomWindow()) {
    waveformValid_ = false;
    needsWaveformRedraw_ = true;
    isDirty_ = true;
    ((AppWindow &)w_).SetDirty();
  }
}

void SampleSlicesView::startPreview() {
  if (!hasInstrumentSample()) {
    return;
  }

  stopPreview();

  uint8_t note = static_cast<uint8_t>(SampleInstrument::SliceNoteBase +
                                      sliceIndexVar_.GetInt());
  Player::GetInstance()->PlayNote(static_cast<unsigned short>(instrumentIndex_),
                                  PreviewChannel, note, 0x7F);
  previewNote_ = note;
  previewActive_ = true;
}

void SampleSlicesView::stopPreview() {
  if (!previewActive_) {
    return;
  }
  Player::GetInstance()->StopNote(static_cast<unsigned short>(instrumentIndex_),
                                  PreviewChannel);
  previewActive_ = false;
}

void SampleSlicesView::handleSliceSelectionChange() {
  bool graphFocused = (GetFocus() == &graphField_);
  updateSliceSelectionFromInstrument();
  if (updateZoomWindow()) {
    waveformValid_ = false;
  }
  needsWaveformRedraw_ = true;
  isDirty_ = true;
  buildFieldLayout();
  if (graphFocused) {
    SetFocus(&graphField_);
  }
}

int32_t SampleSlicesView::sliceToPixel(uint32_t start) const {
  if (sampleSize_ == 0 || viewEnd_ <= viewStart_) {
    return -1;
  }
  if (start < viewStart_ || start >= viewEnd_) {
    return -1;
  }
  uint32_t clamped = std::min(
      start, sampleSize_ > 0 ? sampleSize_ - 1 : static_cast<uint32_t>(0));
  uint32_t viewSpan = viewEnd_ - viewStart_;
  uint32_t rel = clamped - viewStart_;
  int32_t local = static_cast<int32_t>(
      (static_cast<uint64_t>(rel) * (SliceBitmapWidth - 2)) / viewSpan);
  return SliceXOffset + 1 + local;
}

uint32_t SampleSlicesView::selectedSliceStart() {
  int32_t start = sliceStartVar_.GetInt();
  if (start < 0) {
    return 0;
  }
  return static_cast<uint32_t>(start);
}

bool SampleSlicesView::hasInstrumentSample() const {
  return instrument_ && instrument_->GetSampleIndex() >= 0 && sampleSize_ > 0;
}
