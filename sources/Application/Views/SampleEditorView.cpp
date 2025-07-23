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
#include "UIController.h"
#include <cmath>
#include <cstdint>

SampleEditorView::SampleEditorView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), currentInstrument_(NULL), forceRedraw_(false),
      isPlaying_(false), isSingleCycle_(false), playKeyHeld_(false),
      waveformCacheValid_(false), playbackPosition_(0.0f),
      playbackStartFrame_(0) {
  // Initialize waveform cache to zero
  memset(waveformCache_, 0, sizeof(waveformCache_));

  // Clear the buffer
  BitmapGraphics *gfx = BitmapGraphics::GetInstance();

  // TODO: enable after fixing crash on advance
  // gfx->clearBuffer(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT);

  addAllFields();
}

SampleEditorView::~SampleEditorView() {}

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

  // Force redraw of waveform
  forceRedraw_ = true;

  // Invalidate cache on focus to ensure we always have the right waveform
  updateWaveformCache();

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
    // Add waveform display field
    position._y = 2;
    position._x = 0; // start at the left edge of the window
    waveformField_.emplace_back(position, BITMAPWIDTH, BITMAPHEIGHT,
                                bitmapBuffer_, 0xFFFF, 0x0000);
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

  // TODO: enable after fixing crash on advance
  // Update the waveform display
  // updateWaveformDisplay();
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
        updateWaveformDisplay();
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
        updateWaveformDisplay();
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
          uint32_t start = 0;
          uint32_t end = sampleSize - 1;

          Variable *startVar =
              currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
          if (startVar)
            start = startVar->GetInt();

          Variable *endVar =
              currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
          if (endVar)
            end = endVar->GetInt();

          // Ensure parameters are within valid range
          if (start >= sampleSize)
            start = sampleSize - 1;
          if (end >= sampleSize)
            end = sampleSize - 1;
          if (start > end)
            start = end;

          // Get the current frame count since playback started
          uint32_t currentFrame = AppWindow::GetAnimationFrameCounter();
          uint32_t elapsedFrames = currentFrame - playbackStartFrame_;

          // Get the sample duration in seconds
          float duration = currentInstrument_->GetLengthInSec();

          // Calculate the normalized playback position (0.0 to 1.0) based on
          // full sample duration
          float normalizedPos =
              (float)elapsedFrames / (duration * SCREEN_REDRAW_RATE);

          // Calculate the position in the sample, accounting for the start
          // position
          float samplePos = start + normalizedPos * sampleSize;

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
    updateWaveformDisplay();
    forceRedraw_ = false;
  }

  GUITextProperties props;
  drawBattery(props);
  drawPowerButtonUI(props);
}

void SampleEditorView::Update(Observable &o, I_ObservableData *d) {
  // When any of our observed variables change, force a redraw of the waveform
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

void SampleEditorView::updateWaveformDisplay() {
  // Clear the bitmap buffer
  memset(bitmapBuffer_, 0, BITMAPWIDTH * BITMAPHEIGHT / 8);

  // Draw a border around the waveform display
  BitmapGraphics *gfx = BitmapGraphics::GetInstance();
  gfx->drawRect(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, 0, 0, BITMAPWIDTH - 1,
               BITMAPHEIGHT - 1, false, true);

  // Check if we have a valid instrument
  if (!currentInstrument_) {
    // No instrument, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
    return;
  }

  // Get sample size directly from the instrument
  int sampleSize = currentInstrument_->GetSampleSize();
  if (sampleSize <= 0) {
    // No sample data, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
    Trace::Debug("No sample data, just update the bitmap");
    return;
  }

  // Get the sample data from the instrument
  int sampleIndex = currentInstrument_->GetSampleIndex();
  if (sampleIndex < 0) {
    // No valid sample index, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
    return;
  }

  // Get the sample source from the sample pool
  SamplePool *pool = SamplePool::GetInstance();
  SoundSource *source = pool->GetSource(sampleIndex);
  if (!source) {
    // No valid source, just update the bitmap and return
    waveformField_[0].SetBitmap(bitmapBuffer_);
    return;
  }

  // Get sample parameters
  int start = 0;
  int end = sampleSize - 1;
  int loopStart = 0;
  int loopMode = 0;

  Variable *startVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentStart);
  if (startVar)
    start = startVar->GetInt();

  Variable *endVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentEnd);
  if (endVar)
    end = endVar->GetInt();

  Variable *loopStartVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopStart);
  if (loopStartVar)
    loopStart = loopStartVar->GetInt();

  Variable *loopModeVar =
      currentInstrument_->FindVariable(FourCC::SampleInstrumentLoopMode);
  if (loopModeVar)
    loopMode = loopModeVar->GetInt();

  // Ensure parameters are within valid range
  if (start >= sampleSize) {
    start = sampleSize - 1;
  }
  if (end >= sampleSize) {
    end = sampleSize - 1;
  }
  if (loopStart >= sampleSize) {
    loopStart = sampleSize - 1;
  }

  // Draw the waveform from the cache
  int centerY = BITMAPHEIGHT / 2;
  for (int x = 0; x < WAVEFORM_CACHE_SIZE; x++) {
    int pixelHeight = waveformCache_[x];
   
    // Always draw at least a 1-pixel line for visibility, even for very quiet
    // signals
    if (pixelHeight < 1) {
      // For very quiet signals, draw a single pixel line      
      gfx->drawLine(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, x, centerY, x,
                   centerY, true);
    } else {
      // For non-zero signals, draw the full height
      gfx->drawLine(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, x,
                   centerY - pixelHeight, x, centerY + pixelHeight, true);
    }
  }

  // Get the full sample range for proper scaling of markers
  int fullSampleSize = currentInstrument_->GetSampleSize();

  // Calculate positions for start and end markers based on their actual values
  // Map the start position to the bitmap width - start as a fraction of
  // fullSampleSize
  int startX = 1 + (int)(((float)start / fullSampleSize) * (BITMAPWIDTH - 2));
  if (startX < 1)
    startX = 1;
  if (startX >= BITMAPWIDTH - 1)
    startX = BITMAPWIDTH - 2;

  // Draw the start marker line
  gfx->drawLine(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, startX, 1, startX,
               BITMAPHEIGHT - 2, true);

  // Map the end position to the bitmap width - end as a fraction of
  // fullSampleSize
  int endX = 1 + (int)(((float)end / fullSampleSize) * (BITMAPWIDTH - 2));
  if (endX < 1)
    endX = 1;
  if (endX >= BITMAPWIDTH - 1)
    endX = BITMAPWIDTH - 2;

  // Draw the end marker line
  gfx->drawLine(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, endX, 1, endX,
               BITMAPHEIGHT - 2, true);

  // First playhead drawing section - removed

  // Draw markers for loop points if loop mode is enabled
  // draw loop start marker as dashed line
  if (loopMode > 0) {
    // Calculate x position for loop start using the same scaling as start/end
    // markers - loopStart as a fraction of fullSampleSize
    int loopX =
        1 + (int)(((float)loopStart / fullSampleSize) * (BITMAPWIDTH - 2));
    if (loopX >= 1 && loopX < BITMAPWIDTH - 1) {
      // Draw a dashed vertical line for loop start
      for (int y = 1; y < BITMAPHEIGHT - 2; y += 3) {
        // Draw 2-pixel dash, then 1-pixel gap
        gfx->setPixel(bitmapBuffer_, BITMAPWIDTH, loopX, y, true);
        gfx->setPixel(bitmapBuffer_, BITMAPWIDTH, loopX, y + 1, true);
      }
    }
  }

  // Draw the playhead indicator if the sample is playing
  if (isPlaying_) {
    // Calculate the x position for the playhead using the normalized position
    // (0.0-1.0) Map the normalized position directly to bitmap width
    int playheadX = 1 + (int)(playbackPosition_ * (BITMAPWIDTH - 2));

    // Ensure the playhead stays within bounds
    if (playheadX < 1)
      playheadX = 1;
    if (playheadX >= BITMAPWIDTH - 1)
      playheadX = BITMAPWIDTH - 2;

    // Draw a thick vertical line for better visibility
    for (int offset = -1; offset <= 1; offset++) {
      int x = playheadX + offset;
      if (x >= 1 && x < BITMAPWIDTH - 1) {
        gfx->drawLine(bitmapBuffer_, BITMAPWIDTH, BITMAPHEIGHT, x, 1, x,
                     BITMAPHEIGHT - 2, true);
      }
    }

    // Draw a triangle at the top of the playhead
    for (int y = 0; y < 3; y++) {
      for (int x = -y; x <= y; x++) {
        int px = playheadX + x;
        int py = y;
        if (px >= 1 && px < BITMAPWIDTH - 1 && py < BITMAPHEIGHT - 2) {
          gfx->setPixel(bitmapBuffer_, BITMAPWIDTH, px, py, true);
        }
      }
    }
  }

  // Update the bitmap field and request redraw
  if (waveformField_.size() > 0) {
    waveformField_[0].SetBitmap(bitmapBuffer_);
    SetDirty(true);
    // Force immediate redraw of the view
    if (isPlaying_) {
      Redraw();
    }
  }
}