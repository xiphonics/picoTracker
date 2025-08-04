/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SampleEditorView.h"
#include "Application/AppWindow.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Config.h"
#include "Application/Utils/char.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIBitmapField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "Foundation/Types/Types.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "System/Display/BitmapGraphics.h"
#include "System/Profiler/Profiler.h"
#include "UIController.h"
#include <cmath>
#include <cstdint>

SampleEditorView::SampleEditorView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), currentInstrument_(NULL), forceRedraw_(false),
      isPlaying_(false), isSingleCycle_(false), playKeyHeld_(false),
      waveformCacheValid_(false), playbackPosition_(0.0f),
      playbackStartFrame_(0), lastAnimationTime_(0),
      sys_(System::GetInstance()) {
  // Initialize cached sample parameters
  start_ = 0;
  end_ = 0;
  loopStart_ = 0;
  loopMode_ = 0;
  // Initialize waveform cache to zero
  memset(waveformCache_, 0, sizeof(waveformCache_));

#ifdef ADV
  const int scale = 2;
  const int scaled_width = BITMAPWIDTH * scale;
  const int scaled_height = BITMAPHEIGHT * scale;
  scaledBitmapBuffer_ = new uint8_t[scaled_width * scaled_height];
  // Clear the buffer
  memset(scaledBitmapBuffer_, 0, scaled_width * scaled_height);
#else
  // Clear the buffer
  BitmapGraphics *gfx = BitmapGraphics::GetInstance();
  gfx->clearBuffer(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT);
#endif

  addAllFields();
}

SampleEditorView::~SampleEditorView() {
#ifdef ADV
  delete[] scaledBitmapBuffer_;
#endif
}

SampleInstrument *SampleEditorView::getCurrentSampleInstrument() {
  int id = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(id);

  // Check if this is a sample instrument
  if (instr && instr->GetType() == IT_SAMPLE) {
    return static_cast<SampleInstrument *>(instr);
  }

  return nullptr;
}

void SampleEditorView::OnFocus() {
  // Update the current instrument reference
  currentInstrument_ = getCurrentSampleInstrument();

  if (currentInstrument_ == nullptr) {
    const char *newSampleFile = viewData_->sampleEditorFilename.c_str();
    // Load the sample using this filename...
    loadSample(newSampleFile);

    // Clear the field so it's not used again
    viewData_->sampleEditorFilename.clear();
  }

  // Update cached sample parameters
  updateSampleParameters();

  // Force redraw of waveform
  forceRedraw_ = true;

  // Invalidate cache on focus to ensure we always have the right waveform
  updateWaveformCache();

  // make sure we do initial draw of the waveform into bitmap for display
  updateWaveformDisplay();

  FieldView::OnFocus();
  addAllFields();
}

void SampleEditorView::addAllFields() {
  // We currently have no way to update fields with the variable they are
  // assinged so instead we need to first clear out all the previous fields
  // and then re-add them just like we do on the InstrumentView
  fieldList_.clear();
  bigHexVarField_.clear();
  intVarField_.clear();
  actionField_.clear();
  waveformField_.clear();
  nameTextField_.clear();
  nameVariables_.clear();
  // no need to clear staticField_ as they are not added to fieldList_

  GUIPoint position = GetAnchor();

  // update the other fields using the current instrument just like we do
  // initially Add sample parameters if we have a valid instrument
  if (currentInstrument_) {
    // TODO: enable after fixing crash on advance

    // Add waveform display field
    position._y = 2;
    position._x = 0; // start at the left edge of the window
#ifdef ADV
    const int scale = 2;
    const int scaled_width = BITMAPWIDTH * scale;
    const int scaled_height = BITMAPHEIGHT * scale;
    waveformField_.emplace_back(position, scaled_width, scaled_height,
                                scaledBitmapBuffer_, 0xFFFF, 0x0000);
#else
    waveformField_.emplace_back(position, BITMAPWIDTH, BITMAPHEIGHT,
                                bitmapBuffer_, 0xFFFF, 0x0000);
#endif
    fieldList_.insert(fieldList_.end(), &(*waveformField_.rbegin()));

    int sampleSize = currentInstrument_->GetSampleSize();

    position._y = 12; // offset enough for bitmap field
    position._x = 5;

    nameVariables_.emplace_back(currentInstrument_);
    Variable &nameVar = *nameVariables_.rbegin();

    auto label =
        etl::make_string_with_capacity<MAX_UITEXTFIELD_LABEL_LENGTH>("name: ");

    // Use an empty default name - we don't want to populate with sample
    // filename The display name will still be shown on the phrase screen via
    // GetDisplayName()
    etl::string<MAX_INSTRUMENT_NAME_LENGTH> defaultName;

    nameTextField_.emplace_back(nameVar, position, label,
                                FourCC::InstrumentName, defaultName);
    fieldList_.insert(fieldList_.end(), &(*nameTextField_.rbegin()));

    position._y += 1;

    Variable *startVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
    if (startVar) {
      bigHexVarField_.emplace_back(position, *startVar, 7, "start: %7.7X", 0,
                                   sampleSize - 1, 16);
      fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
      (*bigHexVarField_.rbegin()).AddObserver(*this);
    }

    // Add loop start control
    position._y += 1;
    Variable *loopStartVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopStart);
    if (loopStartVar) {
      bigHexVarField_.emplace_back(position, *loopStartVar, 7,
                                   "loop start: %7.7X", 0, sampleSize - 1, 16);
      fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
      (*bigHexVarField_.rbegin()).AddObserver(*this);
    }

    // Add end position control
    position._y += 1;
    Variable *endVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
    if (endVar) {
      bigHexVarField_.emplace_back(position, *endVar, 7, "loop end: %7.7X", 0,
                                   sampleSize - 1, 16);
      fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
      (*bigHexVarField_.rbegin()).AddObserver(*this);
    }

    // Add loop mode control
    position._y += 1;
    Variable *loopModeVar =
        currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopMode);
    if (loopModeVar) {
      intVarField_.emplace_back(position, *loopModeVar, "loop mode: %s", 0, 2,
                                1, 1);
      fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
      (*intVarField_.rbegin()).AddObserver(*this);
    }
  }
}

void SampleEditorView::ProcessButtonMask(unsigned short mask, bool pressed) {
  // Check for key release events
  if (!pressed) {
    // Check if play key was released (exactly like ImportView approach)
    if (playKeyHeld_ && !(mask & EPBM_PLAY)) {
      // Play key no longer pressed so should stop playback
      playKeyHeld_ = false;

      if (Player::GetInstance()->IsPlaying()) {
        // Stop playback regardless of whether it's regular or looping
        Player::GetInstance()->StopStreaming();
        isPlaying_ = false;

        // Force redraw to remove the playhead
        forceRedraw_ = true;
      }
      return;
    }
  }

  // If not pressed, let parent handle it and return
  if (!pressed) {
    FieldView::ProcessButtonMask(mask, pressed);
    return;
  }

  if (mask & EPBM_NAV) {
    if (mask & EPBM_LEFT) {
      // Go back to Instrument view with NAV+LEFT
      ViewType vt = VT_INSTRUMENT;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
      return;
    }
    // For other NAV combinations, let parent handle it
    FieldView::ProcessButtonMask(mask, pressed);
    return;
  } else if (mask & EPBM_PLAY) {
    // Set flag to track that play key is being held down (like in ImportView)
    playKeyHeld_ = true;

    // Start sample playback if we have a valid instrument
    if (currentInstrument_ && !isPlaying_) {
      // Get the sample file name from the instrument's variable
      Variable *sampleVar =
          currentInstrument_->FindVariable(FourCC::SampleInstrumentSample);

      if (sampleVar && sampleVar->GetInt() >= 0) {
        // Get the sample filename from the variable
        etl::string<MAX_INSTRUMENT_NAME_LENGTH> sampleFileName =
            sampleVar->GetString();

        // Get sample size to check if it's a single cycle waveform
        uint32_t sampleSize = currentInstrument_->GetSampleSize();
        isSingleCycle_ = (sampleSize <= SINGLE_CYCLE_MAX_SAMPLE_SIZE);

        Trace::Debug("DEBUG: Starting playback of sample '%s' (size=%d, "
                     "singleCycle=%s)\n",
                     sampleFileName.c_str(), sampleSize,
                     isSingleCycle_ ? "true" : "false");

        // Reset playback state
        isPlaying_ = true;

        // Get the start position which is where playback will begin
        Variable *startVar =
            currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
        uint32_t startSample = startVar->GetInt();
        if (startVar && startSample < sampleSize) {
          // Initialize normalized playback position (0.0 - 1.0)
          playbackPosition_ = (float)startSample / sampleSize;
        } else {
          playbackPosition_ = 0.0f;
        }

        // Store the current animation frame as our start frame
        playbackStartFrame_ = AppWindow::GetAnimationFrameCounter();
        lastAnimationTime_ = sys_->Millis();
        forceRedraw_ = true;

        // If something is already playing, stop it first
        if (Player::GetInstance()->IsPlaying()) {
          Player::GetInstance()->StopStreaming();
        }

        // First change to the current project directory
        auto fs = FileSystem::GetInstance();
        fs->chdir("projects");

        // Get the current project name from view data
        if (viewData_ && viewData_->project_) {
          char projectName[MAX_PROJECT_NAME_LENGTH + 1];
          viewData_->project_->GetProjectName(projectName);

          // Change to the project directory
          if (fs->chdir(projectName)) {
            // Change to the samples directory
            fs->chdir("samples");
          } else {
            Trace::Error("SampleEditorView: Failed to chdir to project dir: %s",
                         projectName);
          }
        } else {
          Trace::Error("SampleEditorView: No project data available");
          return;
        }

        // Start playing the sample with just the filename
        if (isSingleCycle_) {
          Player::GetInstance()->StartLoopingStreaming(sampleFileName.c_str());
        } else {
          // Start playback from the specified start position
          Player::GetInstance()->StartStreaming(sampleFileName.c_str(),
                                                startSample);
        }

        isPlaying_ = true;

        // Force redraw to show the playhead
        forceRedraw_ = true;
      }
    }
    return;
  }

  // For all other button presses, let the parent class handle navigation
  FieldView::ProcessButtonMask(mask, pressed);
}

void SampleEditorView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title
  char titleString[SCREEN_WIDTH];
  strcpy(titleString, "Sample Edit");

  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, titleString, props);

  FieldView::Redraw();

  SetColor(CD_NORMAL);
}

void SampleEditorView::AnimationUpdate() {
  // Update playhead position if sample is playing
  if (isPlaying_ && currentInstrument_) {
    // Check if the Player is still playing the sample
    if (!Player::GetInstance()->IsPlaying()) {
      // Playback has stopped (reached the end of non-looping sample)
      isPlaying_ = false;
      playbackPosition_ = 0;
      forceRedraw_ = true;
      Trace::Debug("DEBUG: Playback stopped, resetting playhead\n");
    } else {
      // Get the current instrument
      SampleInstrument *currentInstrument_ = getCurrentSampleInstrument();
      if (currentInstrument_) {
        uint32_t sampleSize = currentInstrument_->GetSampleSize();
        if (sampleSize > 0) {
          // Get the start and end positions for the active sample range
          uint32_t start = start_;
          uint32_t end = end_;

          // Calculate the time elapsed since the last animation frame
          uint32_t currentTime = sys_->Millis();
          uint32_t elapsedTime = currentTime - lastAnimationTime_;
          lastAnimationTime_ = currentTime;

          // Get the sample duration in milliseconds
          float durationMs = currentInstrument_->GetLengthInSec() * 1000.0f;

          // Calculate the normalized playback position increment
          float positionIncrement = (float)elapsedTime / durationMs;

          // Update the playback position
          playbackPosition_ += positionIncrement;

          // Calculate the position in the sample
          float samplePos = playbackPosition_ * sampleSize;

          // Check if we've reached the end
          if (samplePos >= end || samplePos >= sampleSize) {
            samplePos = end;
            isPlaying_ = false;
          }

          // Update position (normalized to full sample range)
          playbackPosition_ = samplePos / sampleSize;
          forceRedraw_ = true;
        }
      }
    }
  }

  // Check if we need to update the waveform display
  if (forceRedraw_) {

    // TODO: enable after fixing crash on advance
    // Update the waveform display
    updateWaveformDisplay();
    forceRedraw_ = false;
  }

  GUITextProperties props;
  drawBattery(props);
  drawPowerButtonUI(props);
}

void SampleEditorView::Update(Observable &o, I_ObservableData *d) {
  // When any of our observed variables change, update the cached parameters
  updateSampleParameters();
  // Then force a redraw of the waveform
  forceRedraw_ = true;
}

void SampleEditorView::updateWaveformCache() {
  if (!currentInstrument_) {
    waveformCacheValid_ = false;
    return;
  }

  SamplePool *pool = SamplePool::GetInstance();
  SoundSource *source = pool->GetSource(currentInstrument_->GetSampleIndex());

  if (!source) {
    waveformCacheValid_ = false;
    return;
  }

  int sampleSize = source->GetSize(0);
  short *sampleBuffer = (short *)source->GetSampleBuffer(0);

  if (!sampleBuffer || sampleSize == 0) {
    waveformCacheValid_ = false;
    return;
  }

  float samplesPerPixel = (float)sampleSize / (WAVEFORM_CACHE_SIZE - 2);
  int maxHeight = (BITMAPHEIGHT / 2) - 2;

  // First, find the peak amplitude of the sample to determine scaling
  short peakAmplitude = 0;
  for (int i = 0; i < sampleSize; i++) {
    short sampleValue = abs(sampleBuffer[i]);
    if (sampleValue > peakAmplitude) {
      peakAmplitude = sampleValue;
    }
  }

  // Choose a scaling factor based on the peak amplitude
  float scalingFactor = 1.0f;
  if (peakAmplitude < 15000) {
    scalingFactor = 3.0f;
  } else if (peakAmplitude < 25000) {
    scalingFactor = 1.5f;
  }
  Trace::Debug("== Wav samples:%d, peak:%d", sampleSize, peakAmplitude);

  // Now, calculate the RMS for each column and apply the chosen scaling
  for (int x = 0; x < WAVEFORM_CACHE_SIZE; x++) {
    int startSampleIndex = (int)(x * samplesPerPixel);
    int endSampleIndex = (int)((x + 1) * samplesPerPixel);

    if (endSampleIndex <= startSampleIndex) {
      endSampleIndex = startSampleIndex + 1;
    }

    if (startSampleIndex < 0)
      startSampleIndex = 0;
    if (endSampleIndex >= sampleSize)
      endSampleIndex = sampleSize - 1;

    double sumSquares = 0.0;
    int sampleCount = 0;

    for (int i = startSampleIndex; i <= endSampleIndex; i++) {
      float normalizedSample = sampleBuffer[i] / 32768.0f;
      sumSquares += normalizedSample * normalizedSample;
      sampleCount++;
    }

    float rmsValue = (sampleCount > 0) ? sqrt(sumSquares / sampleCount) : 0.0f;
    float scaledRms = rmsValue * scalingFactor;

    // Clamp the value to prevent overflow
    if (scaledRms > 1.0f) {
      scaledRms = 1.0f;
    }

    waveformCache_[x] = (uint8_t)(scaledRms * maxHeight);
  }

  waveformCacheValid_ = true;
}

void SampleEditorView::updateSampleParameters() {
  if (!currentInstrument_) {
    return;
  }

  int sampleSize = 0;
  if (tempSampleSize_ > 0) {
    // --- use the temporary sample's size ---
    sampleSize = tempSampleSize_;
  } else if (currentInstrument_) {
    // --- Fallback to the saved instrument's sample size ---
    sampleSize = currentInstrument_->GetSampleSize();
  } else {
    return; // No sample loaded
  }

  Variable *startVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
  if (startVar)
    start_ = startVar->GetInt();

  Variable *endVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
  if (endVar)
    end_ = endVar->GetInt();

  Variable *loopStartVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopStart);
  if (loopStartVar)
    loopStart_ = loopStartVar->GetInt();

  Variable *loopModeVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopMode);
  if (loopModeVar)
    loopMode_ = loopModeVar->GetInt();

  // Ensure parameters are within valid range
  if (start_ >= sampleSize)
    start_ = sampleSize - 1;
  if (end_ >= sampleSize)
    end_ = sampleSize - 1;
  if (start_ > end_)
    start_ = end_;
  if (loopStart_ > end_)
    loopStart_ = end_;
  if (loopStart_ < start_)
    loopStart_ = start_;
}

void SampleEditorView::updateWaveformDisplay() {
  Profiler p("updateWaveformDisplay");
  BitmapGraphics *gfx = BitmapGraphics::GetInstance();

#ifdef ADV
  const int scale = 2;
  const int scaled_width = BITMAPWIDTH * scale;
  const int scaled_height = BITMAPHEIGHT * scale;
  uint8_t *buffer = scaledBitmapBuffer_;
  memset(buffer, 0, scaled_width * scaled_height);
#else
  const int scale = 1;
  const int scaled_width = BITMAPWIDTH;
  const int scaled_height = BITMAPHEIGHT;
  uint8_t *buffer = bitmapBuffer_;
  memset(buffer, 0, scaled_width * scaled_height / 8);
#endif

  // Draw a border around the waveform display
#ifdef ADV
  // Draw rectangle using direct buffer access for 8bpp
  memset(buffer, 1, scaled_width); // Top edge
  memset(buffer + (scaled_height - 1) * scaled_width, 1,
         scaled_width); // Bottom edge
  for (int y = 1; y < scaled_height - 1; y++) {
    buffer[y * scaled_width] = 1;                    // Left edge
    buffer[y * scaled_width + scaled_width - 1] = 1; // Right edge
  }
#else
  gfx->drawRect(buffer, scaled_width, scaled_height, 0, 0, scaled_width - 1,
                scaled_height - 1, false, true);
#endif

  // Check if we have a valid instrument
  if (!currentInstrument_ || currentInstrument_->GetSampleSize() <= 0 ||
      currentInstrument_->GetSampleIndex() < 0) {
    if (waveformField_.size() > 0) {
#ifdef ADV
      waveformField_[0].SetBitmap(buffer);
#else
      waveformField_[0].SetBitmap(bitmapBuffer_);
#endif
    }
    return;
  }

  // Draw the waveform from the cache, scaled directly
  {
    Profiler p("updateWaveformDisplay: draw waveform");
    int centerY = scaled_height / 2;
    for (int x = 0; x < WAVEFORM_CACHE_SIZE; x++) {
      int pixelHeight = waveformCache_[x] * scale;
      int scaledX = x * scale;

#ifdef ADV
      if (pixelHeight < 1) {
        // For very quiet signals, draw a single pixel line
        memset(buffer + (centerY * scaled_width) + scaledX, 1, scale);
      } else {
        // For non-zero signals, draw the full height
        int startY = centerY - pixelHeight;
        int endY = centerY + pixelHeight;
        for (int y = startY; y <= endY; y++) {
          memset(buffer + (y * scaled_width) + scaledX, 1, scale);
        }
      }
#else
      if (pixelHeight < 1) {
        // For very quiet signals, draw a single pixel line (or scaled
        // equivalent)
        for (int i = 0; i < scale; i++) {
          gfx->setPixel(buffer, scaled_width, scaledX + i, centerY, true);
        }
      } else {
        // For non-zero signals, draw the full height
        for (int i = 0; i < scale; i++) {
          gfx->drawLine(buffer, scaled_width, scaled_height, scaledX + i,
                        centerY - pixelHeight, scaledX + i,
                        centerY + pixelHeight, true);
        }
      }
#endif
    }
  }

  // Use cached sample parameters
  int sampleSize = currentInstrument_->GetSampleSize();
  int start = start_;
  int end = end_;
  int loopStart = loopStart_;
  int loopMode = loopMode_;

  // Draw markers directly to the final buffer
  {
    Profiler p("updateWaveformDisplay: draw markers");
    int fullSampleSize = currentInstrument_->GetSampleSize();
    int startX =
        (1 + (int)(((float)start / fullSampleSize) * (BITMAPWIDTH - 2))) *
        scale;
#ifdef ADV
    for (int y = 1; y < scaled_height - 2; y++) {
      buffer[y * scaled_width + startX] = 1;
    }
#else
    gfx->drawLine(buffer, scaled_width, scaled_height, startX, 1, startX,
                  scaled_height - 2, true);
#endif

    int endX =
        (1 + (int)(((float)end / fullSampleSize) * (BITMAPWIDTH - 2))) * scale;
#ifdef ADV
    for (int y = 1; y < scaled_height - 2; y++) {
      buffer[y * scaled_width + endX] = 1;
    }
#else
    gfx->drawLine(buffer, scaled_width, scaled_height, endX, 1, endX,
                  scaled_height - 2, true);
#endif

    if (loopMode > 0) {
      int loopX =
          (1 + (int)(((float)loopStart / fullSampleSize) * (BITMAPWIDTH - 2))) *
          scale;
      if (loopX >= 1 && loopX < scaled_width - 1) {
#ifdef ADV
        for (int y = 1; y < scaled_height - 2; y += 3) {
          buffer[y * scaled_width + loopX] = 1;
          buffer[(y + 1) * scaled_width + loopX] = 1;
        }
#else
        for (int y = 1; y < scaled_height - 2; y += 3) {
          gfx->setPixel(buffer, scaled_width, loopX, y, true);
          gfx->setPixel(buffer, scaled_width, loopX, y + 1, true);
        }
#endif
      }
    }
  }

  // Draw the playhead indicator if the sample is playing
  if (isPlaying_) {
    Profiler p("updateWaveformDisplay: draw playhead");
    int playheadX = (int)(playbackPosition_ * (scaled_width - 1));
    if (playheadX < 0)
      playheadX = 0;
    if (playheadX >= scaled_width)
      playheadX = scaled_width - 1;

    // Draw a thick vertical line for better visibility
    for (int offset = -1; offset <= 1; offset++) {
      int x = playheadX + offset;
      if (x >= 0 && x < scaled_width) {
#ifdef ADV
        for (int y = 1; y < scaled_height - 2; y++) {
          buffer[y * scaled_width + x] = 1;
        }
#else
        gfx->drawLine(buffer, scaled_width, scaled_height, x, 1, x,
                      scaled_height - 2, true);
#endif
      }
    }

    // Draw a triangle at the top of the playhead
    for (int y = 0; y < 3 * scale; y++) {
      for (int x = -y; x <= y; x++) {
        int px = playheadX + x;
        int py = y;
        if (px >= 0 && px < scaled_width && py < scaled_height - 2) {
#ifdef ADV
          buffer[py * scaled_width + px] = 1;
#else
          gfx->setPixel(buffer, scaled_width, px, py, true);
#endif
        }
      }
    }
  }

  // Update the bitmap field and request redraw
  if (waveformField_.size() > 0) {
#ifdef ADV
    waveformField_[0].SetBitmap(buffer);
#else
    waveformField_[0].SetBitmap(bitmapBuffer_);
#endif
    SetDirty(true);
    if (isPlaying_) {
      Profiler p("updateWaveformDisplay: REDRAW");
      waveformField_.rbegin()->Draw(w_);
    }
  }
}

short SampleEditorView::chunkBuffer_[512 * 2];

void SampleEditorView::loadSample(const char *filename) {
  // These large arrays are now static, so they are not allocated on the stack.
  static double sumSquares[WAVEFORM_CACHE_SIZE];
  static int samplesInPixel[WAVEFORM_CACHE_SIZE];

  // We must clear them at the start of each call since they are static.
  memset(sumSquares, 0, sizeof(sumSquares));
  memset(samplesInPixel, 0, sizeof(samplesInPixel));

  // Reset temporary sample state
  tempSampleSize_ = 0;
  waveformCacheValid_ = false;

  if (!filename || filename[0] == '\0') {
    Trace::Error("missing sample filename");
    return;
  }

  auto fs = FileSystem::GetInstance();

  // First, navigate to the root projects directory
  fs->chdir("/");
  fs->chdir("projects");
  // Then, navigate into the current project's directory
  if (viewData_ && viewData_->project_) {
    char projectName[MAX_PROJECT_NAME_LENGTH + 1];
    viewData_->project_->GetProjectName(projectName);

    if (fs->chdir(projectName)) {
      // Finally, navigate into the samples subdirectory
      fs->chdir("samples");
    } else {
      Trace::Error("SampleEditorView: Failed to chdir to project dir: %s",
                   projectName);
      // It's good practice to return to the root to avoid being in an unknown
      // state
      fs->chdir("/");
      return; // Abort if we can't find the project directory
    }
  } else {
    Trace::Error(
        "SampleEditorView: No project data available to find samples dir.");
    fs->chdir("/");
    return; // Abort if project data is missing
  }

  I_File *file = FileSystem::GetInstance()->Open(filename, "r");
  if (!file) {
    Trace::Error("SampleEditorView: Failed to open file: %s", filename);
    return;
  }

  // --- 1. Read Header & Get Size ---
  char header[44];
  if (file->Read(header, 44) != 44) {
    Trace::Error("SampleEditorView: Failed to read WAV header from %s",
                 filename);
    file->Close();
    return;
  }

  uint16_t numChannels = *reinterpret_cast<uint16_t *>(&header[22]);
  uint16_t bitsPerSample = *reinterpret_cast<uint16_t *>(&header[34]);
  uint32_t dataChunkSize = *reinterpret_cast<uint32_t *>(&header[40]);

  // Added a check to ensure we don't try to use more channels than our buffer
  // supports
  if (numChannels > 2 || numChannels == 0 || bitsPerSample == 0 ||
      dataChunkSize == 0) {
    Trace::Error("SampleEditorView: Invalid or unsupported WAV header in %s",
                 filename);
    file->Close();
    return;
  }
  int bytesPerFrame = numChannels * (bitsPerSample / 8);
  tempSampleSize_ = dataChunkSize / bytesPerFrame;

  // --- 2. Prepare for Single-Pass Processing ---
  Trace::Log("SAMPLEEDITOR", "Parsing sample: %d frames, %d channels",
             tempSampleSize_, numChannels);
  float samplesPerPixel = (float)tempSampleSize_ / WAVEFORM_CACHE_SIZE;

  short peakAmplitude = 0;
  uint32_t currentFrame = 0;

  const int CHUNK_FRAMES = 512;

  // --- 3. The Single-Pass Read Loop ---
  while (currentFrame < tempSampleSize_) {
    int framesToRead =
        std::min(CHUNK_FRAMES, (int)(tempSampleSize_ - currentFrame));
    int bytesToRead = framesToRead * bytesPerFrame;
    int bytesRead = file->Read(chunkBuffer_, bytesToRead);

    int framesRead = (bytesPerFrame > 0) ? bytesRead / bytesPerFrame : 0;
    if (framesRead == 0) {
      break;
    }

    for (int i = 0; i < framesRead; ++i) {
      short sampleValue = chunkBuffer_[i * numChannels];

      if (abs(sampleValue) > peakAmplitude) {
        peakAmplitude = abs(sampleValue);
      }

      int cacheIndex = (currentFrame + i) / samplesPerPixel;
      if (cacheIndex < WAVEFORM_CACHE_SIZE) {
        float normalizedSample = sampleValue / 32768.0f;
        sumSquares[cacheIndex] += normalizedSample * normalizedSample;
        samplesInPixel[cacheIndex]++;
      }
    }
    currentFrame += framesRead;

    if (bytesRead < bytesToRead) {
      break; // Reached end of file
    }
  }
  file->Close();

  // --- 4. Finalize Waveform Cache ---
  float scalingFactor = (peakAmplitude < 15000)   ? 3.0f
                        : (peakAmplitude < 25000) ? 1.5f
                                                  : 1.0f;
  int maxHeight = (BITMAPHEIGHT / 2) - 2;

  for (int x = 0; x < WAVEFORM_CACHE_SIZE; ++x) {
    if (samplesInPixel[x] > 0) {
      float rms = sqrt(sumSquares[x] / samplesInPixel[x]);
      float scaledRms = std::min(1.0f, rms * scalingFactor);
      waveformCache_[x] = (uint8_t)(scaledRms * maxHeight);
    } else {
      waveformCache_[x] = 0;
    }
  }
  waveformCacheValid_ = true;

  // --- 5. Update UI ---
  updateSampleParameters();
  addAllFields();
  updateWaveformDisplay();
  forceRedraw_ = true;
}