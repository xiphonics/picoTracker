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
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Project.h"
#include "Application/Model/Song.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "System/Console/Trace.h"
#include <algorithm>
#include <nanoprintf.h>

namespace {
constexpr uint16_t PreviewChannel = SONG_CHANNEL_COUNT - 1;
constexpr int32_t SliceXOffset = 0;
constexpr int32_t SliceYOffset = 2 * CHAR_HEIGHT;
} // namespace

SampleSlicesView::SampleSlicesView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), sliceIndexVar_(FourCC::SampleInstrumentSlices, 0),
      sliceStartVar_(FourCC::SampleInstrumentStart, 0),
      autoSliceCountVar_(FourCC::Default, 4), needsFullRedraw_(true),
      instrument_(nullptr), instrumentIndex_(0), sampleSize_(0),
      graphFieldPos_(SliceXOffset, SliceYOffset),
      graphField_(graphFieldPos_, GraphField::BitmapWidth,
                  GraphField::BitmapHeight),
      modalWasOpen_(false), modalClearCount_(0), playKeyHeld_(false),
      previewActive_(false), previewNote_(SampleInstrument::SliceNoteBase),
      sys_(System::GetInstance()), previewStartMs_(0), previewStartSample_(0),
      previewEndSample_(0), previewDurationMs_(0.0f), previewPlayheadSample_(0),
      previewCursorVisible_(false) {
  sliceIndexVar_.AddObserver(*this);
  sliceStartVar_.AddObserver(*this);
  sliceIndexLabel_[0] = '\0';
  zoomLabel_[0] = '\0';
  graphField_.SetShowBaseline(false);
}

SampleSlicesView::~SampleSlicesView() { stopPreview(); }

void SampleSlicesView::Reset() {
  stopPreview();
  instrument_ = nullptr;
  instrumentIndex_ = 0;
  sampleSize_ = 0;
  modalWasOpen_ = false;
  modalClearCount_ = 0;
  playKeyHeld_ = false;
  previewActive_ = false;
  previewNote_ = SampleInstrument::SliceNoteBase;
  previewStartMs_ = 0;
  previewStartSample_ = 0;
  previewEndSample_ = 0;
  previewDurationMs_ = 0.0f;
  previewPlayheadSample_ = 0;
  previewCursorVisible_ = false;
  needsFullRedraw_ = true;
  sliceIndexVar_.SetInt(0, false);
  sliceStartVar_.SetInt(0, false);
  autoSliceCountVar_.SetInt(4, false);
  graphField_.Reset();
  graphField_.SetShowBaseline(false);
  sliceIndexLabel_[0] = '\0';
  zoomLabel_[0] = '\0';
}

void SampleSlicesView::OnFocus() {
  stopPreview();
  instrumentIndex_ = static_cast<int32_t>(viewData_->currentInstrumentID_);
  instrument_ = currentInstrument();
  playKeyHeld_ = false;
  previewActive_ = false;
  needsFullRedraw_ = true;
  sampleSize_ = 0;
  modalWasOpen_ = false;
  modalClearCount_ = 0;
  previewStartMs_ = 0;
  previewStartSample_ = 0;
  previewEndSample_ = 0;
  previewDurationMs_ = 0.0f;
  previewPlayheadSample_ = 0;
  previewCursorVisible_ = false;
  graphField_.Reset();
  graphField_.SetShowBaseline(false);

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
  graphField_.RequestFullRedraw();
  autoSliceCountVar_.SetInt(4, false);
  buildFieldLayout();
  isDirty_ = true;
}

void SampleSlicesView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed) {
    if (playKeyHeld_ && !(mask & EPBM_PLAY)) {
      playKeyHeld_ = false;
      stopPreview();
      graphField_.RequestFullRedraw();
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
      graphField_.RequestFullRedraw();
    }
    return;
  }

  bool graphFocused = (GetFocus() == &graphField_);

  if (graphFocused && (mask == EPBM_LEFT)) {
    int32_t index = sliceIndexVar_.GetInt();
    if (index > 0) {
      sliceIndexVar_.SetInt(index - 1);
    }
    return;
  }
  if (graphFocused && (mask == EPBM_RIGHT)) {
    int32_t index = sliceIndexVar_.GetInt();
    if (index < static_cast<int32_t>(SampleInstrument::MaxSlices) - 1) {
      sliceIndexVar_.SetInt(index + 1);
    }
    return;
  }

  if (graphFocused && (mask & EPBM_ENTER)) {
    uint32_t viewStart = graphField_.ViewStart();
    uint32_t viewEnd = graphField_.ViewEnd();
    uint32_t viewSpan = (viewEnd > viewStart) ? (viewEnd - viewStart) : 0;
    if (viewSpan == 0) {
      updateZoomWindow();
      viewStart = graphField_.ViewStart();
      viewEnd = graphField_.ViewEnd();
      viewSpan = (viewEnd > viewStart) ? (viewEnd - viewStart) : 0;
    }
    if (viewSpan > 0) {
      int32_t delta = 0;
      if (mask & (EPBM_LEFT | EPBM_RIGHT)) {
        // We allow single sample movement at max zoom level
        if (graphField_.ZoomLevel() >= graphField_.MaxZoomLevel()) {
          delta = 1;
        } else {
          delta = static_cast<int32_t>(std::max<uint32_t>(1, viewSpan / 64));
        }
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
  if (needsFullRedraw_) {
    Clear();
  }

  GUITextProperties props;
  GUIPoint titlePos = GetTitlePosition();
  DrawString(titlePos._x, titlePos._y, "Sample Slices", props);

  bool hasModal = HasModalView();
  if (!hasModal) {
    drawWaveform();
    ClearTextRect(0, 9, SCREEN_WIDTH, 3);
  }

  if (hasModal) {
    // Modal text cleanup does not cover the graph bitmap area, so avoid
    // redrawing the graph field frame behind the modal.
    if (GetFocus() == 0 && !fieldList_.empty()) {
      SetFocus(*fieldList_.begin());
    }
    for (auto it = fieldList_.begin(); it != fieldList_.end(); ++it) {
      if (*it == &graphField_) {
        continue;
      }
      (*it)->Draw(w_);
    }
  } else {
    FieldView::Redraw();
  }
  needsFullRedraw_ = false;
}

void SampleSlicesView::AnimationUpdate() {
  if (previewActive_ && previewCursorVisible_ && previewDurationMs_ > 0.0f) {
    uint32_t nowMs = sys_ ? sys_->Millis() : 0;
    uint32_t elapsedMs = nowMs - previewStartMs_;
    if (static_cast<float>(elapsedMs) >= previewDurationMs_) {
      previewCursorVisible_ = false;
      isDirty_ = true;
      ((AppWindow &)w_).SetDirty();
    } else {
      float fraction = static_cast<float>(elapsedMs) / previewDurationMs_;
      uint32_t span = (previewEndSample_ > previewStartSample_)
                          ? (previewEndSample_ - previewStartSample_)
                          : 0;
      previewPlayheadSample_ =
          previewStartSample_ +
          static_cast<uint32_t>(fraction * static_cast<float>(span));
      isDirty_ = true;
      ((AppWindow &)w_).SetDirty();
    }
  }

  GUITextProperties props;
  drawBattery(props);
  drawPowerButtonUI(props);
  bool hasModal = HasModalView();
  if (modalWasOpen_ && !hasModal) {
    graphField_.RequestFullRedraw();
    isDirty_ = true;
    ((AppWindow &)w_).SetDirty();
  }
  modalWasOpen_ = hasModal;
  if (!hasModal && modalClearCount_ > 0) {
    graphField_.RequestFullRedraw();
    isDirty_ = true;
    ((AppWindow &)w_).SetDirty();
    drawWaveform();
    modalClearCount_--;
  }
  if (!hasModal && (previewActive_ || previewCursorVisible_)) {
    drawWaveform();
  }
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
    isDirty_ = true;
    ((AppWindow &)w_).SetDirty();
    break;
  case FourCC::ActionAutoSlice:
    if (instrument_ && instrument_->HasSlicesForPlayback()) {
      MessageBox *mb = MessageBox::Create(*this, "Replace current slices?",
                                          MBBF_YES | MBBF_NO);
      modalClearCount_ = 0;
      clearWaveformRegion();
      // Reopening the same modal can reuse identical text, so invalidate only
      // the previous text cache and let the next redraw resend the full dialog.
      ((AppWindow &)w_).InvalidateTextCache();
      DoModal(mb, ModalViewCallback::create<
                      &SampleSlicesView::AutoSliceConfirmCallback>());
    } else {
      autoSliceEvenly();
    }
    break;
  default:
    break;
  }
}

void SampleSlicesView::buildFieldLayout() {
  for (auto &f : intVarField_) {
    f.RemoveObserver(*this);
  }

  fieldList_.clear();
  intVarField_.clear();
  staticField_.clear();
  actionField_.clear();

  fieldList_.insert(fieldList_.end(), &graphField_);

  GUIPoint position = GetAnchor();
  position._x = 1;
  position._y = 8;
  updateStatusLabels();
  staticField_.emplace_back(position, sliceIndexLabel_);
  fieldList_.insert(fieldList_.end(), &staticField_.back());
  position._x += 21;
  staticField_.emplace_back(position, zoomLabel_);
  fieldList_.insert(fieldList_.end(), &staticField_.back());

  position._x = 12;
  intVarField_.emplace_back(position, autoSliceCountVar_, "%2d", 1,
                            static_cast<int32_t>(SampleInstrument::MaxSlices),
                            1, 4);
  fieldList_.insert(fieldList_.end(), &intVarField_.back());

  position._x += 3;
  actionField_.emplace_back("slice", FourCC::ActionAutoSlice, position);
  fieldList_.insert(fieldList_.end(), &actionField_.back());
  actionField_.back().AddObserver(*this);

  position._y += 4;
  position._x = 1;
  staticField_.emplace_back(position, "RIGHT/LEFT: select slice");
  fieldList_.insert(fieldList_.end(), &staticField_.back());
  position._y += 1;
  staticField_.emplace_back(position, "ENTER+UP/DOWN: coarse move");
  fieldList_.insert(fieldList_.end(), &staticField_.back());
  position._y += 1;
  staticField_.emplace_back(position, "ENTER+RIGHT/LEFT: fine move");
  fieldList_.insert(fieldList_.end(), &staticField_.back());
  position._y += 1;
  staticField_.emplace_back(position, "EDIT+UP/DOWN: zoom in/out");
  fieldList_.insert(fieldList_.end(), &staticField_.back());
  position._y += 1;
  staticField_.emplace_back(position, "PLAY: preview slice");
  fieldList_.insert(fieldList_.end(), &staticField_.back());

  SetFocus(&graphField_);
}

void SampleSlicesView::rebuildWaveform() {
  graphField_.InvalidateWaveform();
  graphField_.BeginRmsBuild();

  if (!instrument_) {
    return;
  }

  if (!refreshSampleSize()) {
    return;
  }
  if (sampleSize_ == 0) {
    return;
  }
  if (graphField_.ViewEnd() <= graphField_.ViewStart()) {
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

  int32_t channels = source->GetChannelCount(0);
  int16_t *samples = static_cast<int16_t *>(source->GetSampleBuffer(0));
  if (!samples) {
    return;
  }

  uint32_t viewStart = graphField_.ViewStart();
  uint32_t viewEnd = graphField_.ViewEnd();
  for (uint32_t i = viewStart; i < viewEnd; ++i) {
    int16_t value = samples[i * channels];
    graphField_.AccumulateRmsSample(i, value);
  }

  graphField_.FinalizeRmsBuild();
}

void SampleSlicesView::drawWaveform() {
  if (!graphField_.WaveformValid()) {
    rebuildWaveform();
  }

  graphField_.SetMarkerCount(SampleInstrument::MaxSlices + 1);
  if (instrument_ && sampleSize_ > 0) {
    for (size_t i = 0; i < SampleInstrument::MaxSlices; ++i) {
      if (!instrument_->IsSliceDefined(i)) {
        graphField_.SetMarker(i, 0, CD_ACCENT, false);
        continue;
      }
      uint32_t start = instrument_->GetSlicePoint(i);
      if (i == 0 && start == 0 && !instrument_->HasSlicesForPlayback()) {
        graphField_.SetMarker(i, 0, CD_ACCENT, false);
        continue;
      }
      ColorDefinition color =
          (static_cast<int32_t>(i) == sliceIndexVar_.GetInt()) ? CD_HILITE2
                                                               : CD_ACCENT;
      graphField_.SetMarker(i, start, color, true);
    }
  } else {
    for (size_t i = 0; i < SampleInstrument::MaxSlices; ++i) {
      graphField_.SetMarker(i, 0, CD_ACCENT, false);
    }
  }

  size_t playheadIndex = SampleInstrument::MaxSlices;
  if (previewCursorVisible_) {
    graphField_.SetMarker(playheadIndex, previewPlayheadSample_, CD_NORMAL,
                          true);
  } else {
    graphField_.SetMarker(playheadIndex, 0, CD_NORMAL, false);
  }

  graphField_.DrawGraph(*this);
}

void SampleSlicesView::clearWaveformRegion() {
  GUIRect rect(graphFieldPos_._x, graphFieldPos_._y,
               graphFieldPos_._x + GraphField::BitmapWidth,
               graphFieldPos_._y + GraphField::BitmapHeight);
  DrawRect(rect, CD_BACKGROUND);
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
  if (index >= static_cast<int32_t>(SampleInstrument::MaxSlices)) {
    index = static_cast<int32_t>(SampleInstrument::MaxSlices) - 1;
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
    graphField_.InvalidateWaveform();
    updateStatusLabels();
    isDirty_ = true;
  }
}

void SampleSlicesView::autoSliceEvenly() {
  if (!instrument_) {
    return;
  }
  if (!refreshSampleSize()) {
    return;
  }
  int32_t count = autoSliceCountVar_.GetInt();
  if (count < 1) {
    return;
  }
  if (count > static_cast<int32_t>(SampleInstrument::MaxSlices)) {
    count = static_cast<int32_t>(SampleInstrument::MaxSlices);
  }
  instrument_->ClearSlices();
  if (count > 1) {
    for (int32_t i = 0; i < count; ++i) {
      uint32_t start =
          (static_cast<uint64_t>(sampleSize_) * static_cast<uint64_t>(i)) /
          static_cast<uint32_t>(count);
      instrument_->SetSlicePoint(static_cast<size_t>(i), start);
    }
  }
  updateSliceSelectionFromInstrument();
  if (updateZoomWindow()) {
    graphField_.InvalidateWaveform();
  }
  graphField_.RequestFullRedraw();
  updateStatusLabels();
  isDirty_ = true;
  ((AppWindow &)w_).SetDirty();
}

bool SampleSlicesView::refreshSampleSize() {
  sampleSize_ = 0;
  if (!instrument_) {
    graphField_.SetSampleSize(0);
    return false;
  }
  SamplePool *pool = SamplePool::GetInstance();
  int32_t sampleIndex = instrument_->GetSampleIndex();
  if (sampleIndex < 0) {
    graphField_.SetSampleSize(0);
    return false;
  }
  SoundSource *source = pool->GetSource(sampleIndex);
  if (!source) {
    graphField_.SetSampleSize(0);
    return false;
  }
  int32_t size = source->GetSize(0);
  sampleSize_ = (size > 0) ? static_cast<uint32_t>(size) : 0;
  graphField_.SetSampleSize(sampleSize_);
  return sampleSize_ > 0;
}

void SampleSlicesView::updateStatusLabels() {
  int32_t sliceIndex = sliceIndexVar_.GetInt();
  if (sliceIndex < 0) {
    sliceIndex = 0;
  }
  npf_snprintf(sliceIndexLabel_, sizeof(sliceIndexLabel_), "slice: %2d",
               sliceIndex);

  uint32_t zoom = 1u;
  uint8_t level = graphField_.ZoomLevel();
  if (level < 31) {
    zoom = 1u << level;
  }
  npf_snprintf(zoomLabel_, sizeof(zoomLabel_), "zoom:%3ux", zoom);
}

void SampleSlicesView::AutoSliceConfirmCallback(View &v, ModalView &dialog) {
  auto &self = static_cast<SampleSlicesView &>(v);
  self.modalClearCount_ = 2;
  self.isDirty_ = true;
  ((AppWindow &)self.w_).SetDirty();
  if (dialog.GetReturnCode() != MBL_YES) {
    return;
  }
  self.autoSliceEvenly();
}

void SampleSlicesView::updateZoomLimits() {
  graphField_.SetSampleSize(sampleSize_);
}

bool SampleSlicesView::updateZoomWindow() {
  bool changed = graphField_.UpdateZoomWindow(selectedSliceStart());
  if (changed) {
    updateStatusLabels();
  }
  return changed;
}

void SampleSlicesView::adjustZoom(int32_t delta) {
  if (sampleSize_ == 0) {
    return;
  }
  if (graphField_.AdjustZoom(delta, selectedSliceStart())) {
    graphField_.InvalidateWaveform();
    graphField_.RequestFullRedraw();
    updateStatusLabels();
    isDirty_ = true;
    ((AppWindow &)w_).SetDirty();
  }
}

void SampleSlicesView::startPreview() {
  if (!hasInstrumentSample()) {
    return;
  }

  stopPreview();

  uint32_t startSample = selectedSliceStart();
  uint32_t endSample = sliceEndForIndex(
      static_cast<size_t>(sliceIndexVar_.GetInt()), startSample);
  if (endSample <= startSample && sampleSize_ > 0) {
    endSample = sampleSize_ - 1;
  }

  uint8_t note = static_cast<uint8_t>(SampleInstrument::SliceNoteBase +
                                      sliceIndexVar_.GetInt());
  if (instrument_ && !instrument_->HasSlicesForPlayback()) {
    // For samples  that have no stored slice points yet, triggering a
    // slice note in that state is interpreted as a pitched note, which makes
    // preview play at the wrong speed because for slices we set the "root note"
    // to C2 for the first slice when we enter the slicer
    Variable *rootNoteVar =
        instrument_->FindVariable(FourCC::SampleInstrumentRootNote);
    note = static_cast<uint8_t>(rootNoteVar->GetInt());
  }

  Player::GetInstance()->PlayNote(static_cast<unsigned short>(instrumentIndex_),
                                  PreviewChannel, note, 0x7F);
  previewNote_ = note;
  previewActive_ = true;
  previewStartSample_ = startSample;
  previewEndSample_ = endSample;
  previewPlayheadSample_ = startSample;

  float durationMs = 0.0f;
  if (instrument_) {
    SamplePool *pool = SamplePool::GetInstance();
    int32_t sampleIndex = instrument_->GetSampleIndex();
    if (sampleIndex >= 0) {
      if (SoundSource *source = pool->GetSource(sampleIndex)) {
        int32_t sampleRate = source->GetSampleRate(note);
        if (sampleRate > 0 && endSample > startSample) {
          uint32_t frames = endSample - startSample;
          durationMs = (static_cast<float>(frames) * 1000.0f) /
                       static_cast<float>(sampleRate);
        }
      }
    }
  }
  previewDurationMs_ = durationMs;
  previewStartMs_ = sys_ ? sys_->Millis() : 0;
  previewCursorVisible_ = (previewDurationMs_ > 0.0f);
}

void SampleSlicesView::stopPreview() {
  if (!previewActive_) {
    return;
  }
  Player::GetInstance()->StopNote(static_cast<unsigned short>(instrumentIndex_),
                                  PreviewChannel);
  previewActive_ = false;
  previewCursorVisible_ = false;
  graphField_.RequestFullRedraw();
}

void SampleSlicesView::handleSliceSelectionChange() {
  bool graphFocused = (GetFocus() == &graphField_);
  updateSliceSelectionFromInstrument();
  if (updateZoomWindow()) {
    graphField_.InvalidateWaveform();
    graphField_.RequestFullRedraw();
  }
  isDirty_ = true;
  buildFieldLayout();
  if (graphFocused) {
    SetFocus(&graphField_);
  }
}

uint32_t SampleSlicesView::selectedSliceStart() {
  int32_t start = sliceStartVar_.GetInt();
  if (start < 0) {
    return 0;
  }
  return static_cast<uint32_t>(start);
}

uint32_t SampleSlicesView::sliceEndForIndex(size_t index,
                                            uint32_t start) const {
  if (!instrument_ || sampleSize_ == 0) {
    return 0;
  }
  uint32_t end = sampleSize_ > 0 ? sampleSize_ - 1 : 0;
  for (size_t i = index + 1; i < SampleInstrument::MaxSlices; ++i) {
    if (!instrument_->IsSliceDefined(i)) {
      continue;
    }
    uint32_t nextStart = instrument_->GetSlicePoint(i);
    if (nextStart > start) {
      end = nextStart;
      break;
    }
  }
  return end;
}

bool SampleSlicesView::hasInstrumentSample() const {
  return instrument_ && instrument_->GetSampleIndex() >= 0 && sampleSize_ > 0;
}
