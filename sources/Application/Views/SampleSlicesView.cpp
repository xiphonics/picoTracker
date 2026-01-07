/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SampleSlicesView.h"

#ifdef CHAR_WIDTH
#undef CHAR_WIDTH
#endif
#ifdef CHAR_HEIGHT
#undef CHAR_HEIGHT
#endif

#include "Application/AppWindow.h"
#include "Application/Instruments/InstrumentBank.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Project.h"
#include "Application/Model/Song.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "System/Console/Trace.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace {
constexpr unsigned short PreviewChannel = SONG_CHANNEL_COUNT - 1;
constexpr int SliceXOffset = 0;
#ifdef ADV
constexpr int SliceYOffset = 2 * CHAR_HEIGHT * 4;
#else
constexpr int SliceYOffset = 2 * CHAR_HEIGHT;
#endif
} // namespace

SampleSlicesView::SampleSlicesView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), sliceIndexVar_(FourCC::SampleInstrumentSlices, 0),
      sliceStartVar_(FourCC::SampleInstrumentStart, 0), waveformValid_(false),
      needsWaveformRedraw_(true), instrument_(nullptr), instrumentIndex_(0),
      sampleSize_(0), playKeyHeld_(false), previewActive_(false),
      previewNote_(SampleInstrument::SliceNoteBase) {
  sliceIndexVar_.AddObserver(*this);
  sliceStartVar_.AddObserver(*this);
  std::memset(waveformCache_, 0, sizeof(waveformCache_));
}

SampleSlicesView::~SampleSlicesView() { stopPreview(); }

void SampleSlicesView::OnFocus() {
  stopPreview();
  instrumentIndex_ = viewData_->currentInstrumentID_;
  instrument_ = currentInstrument();
  playKeyHeld_ = false;
  previewActive_ = false;
  waveformValid_ = false;
  needsWaveformRedraw_ = true;
  sampleSize_ = 0;

  if (instrument_) {
    SamplePool *pool = SamplePool::GetInstance();
    int sampleIndex = instrument_->GetSampleIndex();
    if (sampleIndex >= 0) {
      if (SoundSource *source = pool->GetSource(sampleIndex)) {
        int size = source->GetSize(0);
        sampleSize_ = (size > 0) ? static_cast<uint32_t>(size) : 0;
      }
    }
  }

  sliceIndexVar_.SetInt(0, false);
  updateSliceSelectionFromInstrument();
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
                            static_cast<int>(SliceCount) - 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &intVarField_.back());
  intVarField_.back().AddObserver(*this);

  position._y += 1;
  int maxStart = (sampleSize_ > 0) ? static_cast<int>(sampleSize_ - 1) : 0;
  int minStart = 0;
  if (instrument_) {
    int index = sliceIndexVar_.GetInt();
    if (index > 0) {
      minStart = static_cast<int>(instrument_->GetSlicePoint(index - 1));
    }
  }
  bigHexVarField_.emplace_back(position, sliceStartVar_, 7, "start: %7.7X",
                               minStart, maxStart, 16);
  fieldList_.insert(fieldList_.end(), &bigHexVarField_.back());
  bigHexVarField_.back().AddObserver(*this);

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
  int sampleIndex = instrument_->GetSampleIndex();
  if (sampleIndex < 0) {
    return;
  }

  SoundSource *source = pool->GetSource(sampleIndex);
  if (!source) {
    return;
  }

  int size = source->GetSize(0);
  sampleSize_ = (size > 0) ? static_cast<uint32_t>(size) : 0;
  if (sampleSize_ == 0) {
    return;
  }

  int channels = source->GetChannelCount(0);
  short *samples = static_cast<short *>(source->GetSampleBuffer(0));
  if (!samples) {
    return;
  }

  std::fill(std::begin(waveformCache_), std::end(waveformCache_), 0);
  static int64_t sumSquares[SliceWaveformCacheSize];
  static uint32_t counts[SliceWaveformCacheSize];
  std::fill_n(sumSquares, SliceWaveformCacheSize, int64_t{0});
  std::fill_n(counts, SliceWaveformCacheSize, uint32_t{0});
  float samplesPerPixel =
      std::max(1.0f, static_cast<float>(sampleSize_) / SliceWaveformCacheSize);

  for (uint32_t i = 0; i < sampleSize_; ++i) {
    uint32_t pixel =
        static_cast<uint32_t>(std::floor(i / samplesPerPixel + 0.5f));
    if (pixel >= SliceWaveformCacheSize) {
      pixel = SliceWaveformCacheSize - 1;
    }
    short value = samples[i * channels];
    sumSquares[pixel] += static_cast<int64_t>(value) * value;
    counts[pixel]++;
  }

  for (int i = 0; i < SliceWaveformCacheSize; ++i) {
    if (counts[i] == 0) {
      waveformCache_[i] = 0;
      continue;
    }
    float meanSquare = static_cast<float>(sumSquares[i]) / counts[i];
    float rms = std::sqrt(meanSquare) / 32768.0f;
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

  if (waveformValid_) {
    int centerY = SliceYOffset + SliceBitmapHeight / 2;
    for (int x = 1; x < SliceBitmapWidth - 1; ++x) {
      uint8_t amplitude = waveformCache_[x - 1];
      if (amplitude == 0) {
        continue;
      }
      int startY = centerY - amplitude / 2;
      int endY = startY + amplitude;
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
    int x = sliceToPixel(start);
    if (x < 0) {
      continue;
    }
    ColorDefinition color = (static_cast<int>(i) == sliceIndexVar_.GetInt())
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

  int index = sliceIndexVar_.GetInt();
  if (index < 0) {
    index = 0;
  }
  if (index >= static_cast<int>(SliceCount)) {
    index = static_cast<int>(SliceCount) - 1;
  }

  size_t sliceIndex = static_cast<size_t>(index);
  uint32_t start = instrument_->GetSlicePoint(sliceIndex);
  sliceStartVar_.SetInt(static_cast<int>(start), false);
}

void SampleSlicesView::applySliceStart(uint32_t start) {
  if (!instrument_) {
    return;
  }
  size_t index = static_cast<size_t>(sliceIndexVar_.GetInt());
  instrument_->SetSlicePoint(index, start);
  uint32_t stored = instrument_->GetSlicePoint(index);
  if (stored != start) {
    sliceStartVar_.SetInt(static_cast<int>(stored), false);
  }
}

void SampleSlicesView::startPreview() {
  if (!hasInstrumentSample()) {
    return;
  }

  stopPreview();

  unsigned char note = static_cast<unsigned char>(
      SampleInstrument::SliceNoteBase + sliceIndexVar_.GetInt());
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
  updateSliceSelectionFromInstrument();
  needsWaveformRedraw_ = true;
  isDirty_ = true;
  buildFieldLayout();
}

int SampleSlicesView::sliceToPixel(uint32_t start) const {
  if (sampleSize_ == 0) {
    return -1;
  }
  uint32_t clamped = std::min(
      start, sampleSize_ > 0 ? sampleSize_ - 1 : static_cast<uint32_t>(0));
  float ratio = static_cast<float>(clamped) /
                static_cast<float>(std::max<uint32_t>(1, sampleSize_));
  int local = static_cast<int>(ratio * (SliceBitmapWidth - 2));
  return SliceXOffset + 1 + local;
}

bool SampleSlicesView::hasInstrumentSample() const {
  return instrument_ && instrument_->GetSampleIndex() >= 0 && sampleSize_ > 0;
}
