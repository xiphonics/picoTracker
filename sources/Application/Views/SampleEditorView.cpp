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
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "Foundation/Types/Types.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "System/Profiler/Profiler.h"
#include "UIController.h"
#include <cmath>
#include <cstdint>

SampleEditorView::SampleEditorView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), fullWaveformRedraw_(false), isPlaying_(false),
      isSingleCycle_(false), playKeyHeld_(false), waveformCacheValid_(false),
      playbackPosition_(0.0f), playbackStartFrame_(0), lastAnimationTime_(0),
      sys_(System::GetInstance()), startVar_(FourCC::VarSampleEditStart, 0),
      endVar_(FourCC::VarSampleEditEnd, 0),
      // bit of a hack to use InstrumentName but we never actually persist this
      // in any config file here
      filenameVar_(FourCC::InstrumentName, ""), win(w), redraw_(false) {
  // Initialize waveform cache to zero
  memset(waveformCache_, 0, BITMAPWIDTH * sizeof(uint8_t));
}

SampleEditorView::~SampleEditorView() {}

void SampleEditorView::OnFocus() {
  const auto newSampleFile = viewData_->sampleEditorFilename;
  // Use the passed in filename to fill in our filename variable with the last 4
  // chars (".wav") removed
  filenameVar_.SetString(
      newSampleFile.substr(0, newSampleFile.length() - 4).c_str());
  // Load the sample using this filename and will fill in the wave data cache
  loadSample(newSampleFile, viewData_->sampleEditorProjectList);

  // Update cached sample parameters
  updateSampleParameters();

  // Force redraw of waveform
  fullWaveformRedraw_ = true;

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
  nameTextField_.clear();
  // no need to clear staticField_  as its not added to fieldList_

  GUIPoint position = GetAnchor();

  position._y = 12; // offset enough for waveform display
  position._x = 5;

  auto label =
      etl::make_string_with_capacity<MAX_UITEXTFIELD_LABEL_LENGTH>("name: ");

  nameTextField_.emplace_back(filenameVar_, position, label,
                              FourCC::InstrumentName, filename);
  fieldList_.insert(fieldList_.end(), &(*nameTextField_.rbegin()));

  position._y += 1;
  bigHexVarField_.emplace_back(position, startVar_, 7, "start: %7.7X", 0,
                               tempSampleSize_ - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  // Add end position control
  position._y += 1;

  bigHexVarField_.emplace_back(position, endVar_, 7, "end: %7.7X", 0,
                               tempSampleSize_ - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);
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
        redraw_ = true;
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
      ViewType vt = VT_IMPORT;
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

    // Start sample playback if no already playing
    if (!isPlaying_) {
      // Get the sample file name
      const auto sampleFileName = viewData_->sampleEditorFilename;
      isSingleCycle_ = (tempSampleSize_ <= SINGLE_CYCLE_MAX_SAMPLE_SIZE);

      Trace::Debug("DEBUG: Starting playback of sample '%s' (size=%d, "
                   "singleCycle=%s)\n",
                   sampleFileName.c_str(), tempSampleSize_,
                   isSingleCycle_ ? "true" : "false");

      // Reset playback state
      isPlaying_ = true;

      // Get the start position which is where playback will begin
      uint32_t startSample = startVar_.GetInt();
      if (startSample < tempSampleSize_) {
        // Initialize normalized playback position (0.0 - 1.0)
        playbackPosition_ = (float)startSample / tempSampleSize_;
      } else {
        playbackPosition_ = 0.0f;
      }

      // Store the current animation frame as our start frame
      playbackStartFrame_ = AppWindow::GetAnimationFrameCounter();
      lastAnimationTime_ = sys_->Millis();
      redraw_ = true;

      // If something is already playing, stop it first
      if (Player::GetInstance()->IsPlaying()) {
        Player::GetInstance()->StopStreaming();
      }

      // First change to the current project directory
      if (!goProjectSamplesDir()) {
        Trace::Error("couldnt change to project samples dir!");
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
      // redraw to show the playhead
      redraw_ = true;
    }
    return;
  }

  // For all other button presses, let the parent class handle navigation
  FieldView::ProcessButtonMask(mask, pressed);
}

// Helper to redraw the content of a single column (waveform or just
// background) This function will clear a 1-pixel wide column and redraw the
// waveform in it.
void redrawColumn(View &view, const uint8_t *waveformCache, int x_coord,
                  int x_offset, int y_offset) {
  if (x_coord < 0)
    return; // Invalid coordinate

  GUIRect rrect;

  // 1. Clear the column
  // The Y range for clearing should cover the entire waveform area, including
  // borders
  rrect =
      GUIRect(x_coord, y_offset + 1, x_coord + 1, y_offset + BITMAPHEIGHT - 1);
  view.DrawRect(rrect, CD_BACKGROUND);

  // 2. Redraw the waveform in that column
  int waveform_idx =
      x_coord - x_offset - 1; // Adjust for X_OFFSET and 1-pixel border
  if (waveform_idx >= 0 && waveform_idx < WAVEFORM_CACHE_SIZE) {
    int pixelHeight = waveformCache[waveform_idx];
    if (pixelHeight > 0) {
      int centerY = y_offset + BITMAPHEIGHT / 2;
      int startY = centerY - pixelHeight / 2;
      int endY = startY + pixelHeight;

      // Clamp to inside the border
      if (startY < y_offset + 1)
        startY = y_offset + 1;
      if (endY > y_offset + BITMAPHEIGHT - 2)
        endY = y_offset + BITMAPHEIGHT - 2;

      if (startY <= endY) {
        rrect = GUIRect(x_coord, startY, x_coord + 1, endY);
        view.DrawRect(rrect, CD_NORMAL);
      }
    }
  }
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

  DrawWaveForm();

  // Let the base class draw all the text fields
  FieldView::Redraw();
}

void SampleEditorView::DrawWaveForm() {
  const int X_OFFSET = 0;
#ifdef ADV
  const int Y_OFFSET = 2 * CHAR_HEIGHT * 4;
#else
  const int Y_OFFSET = 2 * CHAR_HEIGHT;
#endif

  GUIRect rrect;

  if (fullWaveformRedraw_) {
    // clear flag immediately to prevent race condition between event triggered
    // from input event and the animation update callback
    fullWaveformRedraw_ = false;
    // Clear the entire waveform area
    rrect = GUIRect(X_OFFSET, Y_OFFSET, X_OFFSET + BITMAPWIDTH,
                    Y_OFFSET + BITMAPHEIGHT);
    DrawRect(rrect, CD_BACKGROUND);

    rrect = GUIRect(X_OFFSET, Y_OFFSET, X_OFFSET + BITMAPWIDTH, Y_OFFSET + 1);
    // Draw borders
    DrawRect(rrect, CD_HILITE1); // Top
    rrect = GUIRect(X_OFFSET, Y_OFFSET + BITMAPHEIGHT - 1,
                    X_OFFSET + BITMAPWIDTH, Y_OFFSET + BITMAPHEIGHT);
    DrawRect(rrect, CD_HILITE1); // Bottom

    // Draw full waveform
    if (waveformCacheValid_) {
      int centerY = Y_OFFSET + BITMAPHEIGHT / 2;
      for (int x = 0; x < WAVEFORM_CACHE_SIZE; x++) {
        int pixelHeight = waveformCache_[x];
        if (pixelHeight > 0) {
          int startY = centerY - pixelHeight / 2;
          int endY = startY + pixelHeight;

          // Clamp to inside the border
          if (startY < Y_OFFSET + 1)
            startY = Y_OFFSET + 1;
          if (endY > Y_OFFSET + BITMAPHEIGHT - 2)
            endY = Y_OFFSET + BITMAPHEIGHT - 2;

          if (startY <= endY) {
            rrect = GUIRect(X_OFFSET + x + 1, startY, X_OFFSET + x + 2, endY);
            DrawRect(rrect, CD_NORMAL);
          }
        }
      }
    }
    // After a full redraw, invalidate last positions to ensure markers are
    // drawn fresh
    last_start_x_ = -1;
    last_end_x_ = -1;
    last_playhead_x_ = -1;
  }

  // --- Incremental Marker Update Logic ---

  // Calculate current marker positions
  const int current_startX =
      X_OFFSET + 1 +
      static_cast<int>((static_cast<float>(start_) / tempSampleSize_) *
                       (BITMAPWIDTH - 2));
  const int current_endX =
      X_OFFSET + 1 +
      static_cast<int>((static_cast<float>(end_) / tempSampleSize_) *
                       (BITMAPWIDTH - 2));
  const int current_playheadX =
      isPlaying_ ? (X_OFFSET + 1 +
                    static_cast<int>(playbackPosition_ * (BITMAPWIDTH - 2)))
                 : -1;

  // Erase old markers if they have moved or disappeared
  if (last_start_x_ != -1 && last_start_x_ != current_startX) {
    redrawColumn(*this, waveformCache_, last_start_x_, X_OFFSET, Y_OFFSET);
  }
  if (last_end_x_ != -1 && last_end_x_ != current_endX) {
    redrawColumn(*this, waveformCache_, last_end_x_, X_OFFSET, Y_OFFSET);
  }
  if (last_playhead_x_ != -1 && last_playhead_x_ != current_playheadX) {
    redrawColumn(*this, waveformCache_, last_playhead_x_, X_OFFSET, Y_OFFSET);
  }

  // Draw new markers
  if (current_startX != -1) { // Only draw if valid
    rrect = GUIRect(current_startX, Y_OFFSET + 2, current_startX + 1,
                    Y_OFFSET + BITMAPHEIGHT - 3);
    DrawRect(rrect, CD_CURSOR);
  }
  if (current_endX != -1) { // Only draw if valid
    rrect = GUIRect(current_endX, Y_OFFSET + 2, current_endX + 1,
                    Y_OFFSET + BITMAPHEIGHT - 3);
    DrawRect(rrect, CD_CURSOR);
  }
  if (current_playheadX != -1) { // Only draw if valid and playing
    rrect = GUIRect(current_playheadX, Y_OFFSET + 2, current_playheadX + 1,
                    Y_OFFSET + BITMAPHEIGHT - 3);
    DrawRect(rrect, CD_CURSOR);
  }

  // Update last known positions
  last_start_x_ = current_startX;
  last_end_x_ = current_endX;
  last_playhead_x_ = current_playheadX;

  // Reset redraw flags
  redraw_ = false;
}

void SampleEditorView::AnimationUpdate() {
  // Update playhead position if sample is playing
  if (isPlaying_) {
    // Check if the Player is still playing the sample
    if (!Player::GetInstance()->IsPlaying()) {
      // Playback has stopped (reached the end of non-looping sample)
      Player::GetInstance()->StopStreaming();
      isPlaying_ = false;
      playbackPosition_ = 0;
      // forceRedraw_ = true;
      Trace::Debug("DEBUG: Playback stopped, resetting playhead\n");
    } else {
      // Calculate the time elapsed since the last animation frame
      uint32_t currentTime = sys_->Millis();
      uint32_t elapsedTime = currentTime - lastAnimationTime_;
      lastAnimationTime_ = currentTime;

      // Get the sample duration in milliseconds, assuming 44.1kHz sample rate
      float durationMs = (float)tempSampleSize_ / 44100.0f * 1000.0f;

      // Calculate the normalized playback position increment
      float positionIncrement = (float)elapsedTime / durationMs;

      // Update the playback position
      playbackPosition_ += positionIncrement;

      // Calculate the position in the sample
      float samplePos = playbackPosition_ * tempSampleSize_;

      // Check if we've reached the end
      if (samplePos >= end_ || samplePos >= tempSampleSize_) {
        samplePos = end_;
        Trace::Debug("Playback stopped at end!");
      }

      // Update position (normalized to full sample range)
      playbackPosition_ = samplePos / tempSampleSize_;
      redraw_ = true;
    }
  }

  // Check if we need to update the waveform display
  if (redraw_) {
    // Update the waveform display
    DrawWaveForm();
    redraw_ = false;
  }

  GUITextProperties props;
  drawBattery(props);
  drawPowerButtonUI(props);
}

void SampleEditorView::Update(Observable &o, I_ObservableData *d) {
  // When any of our observed variables change, update the cached parameters
  updateSampleParameters();
  // Then do a redraw of the waveform markers only
  redraw_ = true;
}

void SampleEditorView::updateSampleParameters() {

  int sampleSize = 0;
  if (tempSampleSize_ > 0) {
    // --- use the temporary sample's size ---
    sampleSize = tempSampleSize_;
  }

  start_ = startVar_.GetInt();
  end_ = endVar_.GetInt();

  // Ensure parameters are within valid range
  if (start_ >= sampleSize)
    start_ = sampleSize - 1;
  if (end_ >= sampleSize)
    end_ = sampleSize - 1;
  if (start_ > end_)
    start_ = end_;
}

short SampleEditorView::chunkBuffer_[512 * 2];

// Load sample from the current projects samples subdir
// ONLY filename for samples subidr (not full path!!) is supported for now
void SampleEditorView::loadSample(
    const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> filename,
    bool isProjectSampleFile) {

  // Reset temporary sample state
  tempSampleSize_ = 0;
  waveformCacheValid_ = false;
  static int64_t sumSquares[WAVEFORM_CACHE_SIZE];
  memset(sumSquares, 0, sizeof(sumSquares));

  if (filename.empty()) {
    Trace::Error("missing sample filename");
    return;
  }

  if (isProjectSampleFile) {
    // First, navigate to the root projects samples subdir directory
    if (!goProjectSamplesDir()) {
      Trace::Error("couldn't change to project samples dir!");
      return;
    }
  } else {
    // do nothing, we rely on the current directory being correctly set for the
    // sample file we are trying to edit
  }

  I_File *file = FileSystem::GetInstance()->Open(filename.c_str(), "r");
  if (!file) {
    Trace::Error("SampleEditorView: Failed to open file: %s", filename);
    return;
  }
  Trace::Log("SAMPLEEDITOR", "Loaded for parsing: %s", filename);

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
  Trace::Log("SAMPLEEDITOR", "samples/p/p: %f", samplesPerPixel);

  short peakAmplitude = 0;
  uint32_t currentFrame = 0;

  const int CHUNK_FRAMES = 512;

  uint32_t waveCacheValueAccum = 0;
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
        // Accumulate sum of squares using integer arithmetic for performance.
        // Floating point conversion will be done once for each cache entry
        // later.
        sumSquares[cacheIndex] += (int64_t)sampleValue * sampleValue;
      }
    }
    currentFrame += framesRead;

    if (bytesRead < bytesToRead) {
      break; // Reached end of file
    }
  }
  file->Close();

  // --- 4. Finalize Waveform Cache ---
  for (int x = 0; x < WAVEFORM_CACHE_SIZE; ++x) {
    if (samplesPerPixel >= 1) {
      // Calculate the Root Mean Square (RMS)
      // 1. Calculate mean of squares from our accumulated integer values.
      float mean_square = (float)sumSquares[x] / samplesPerPixel;

      // 2. Take the square root.
      float rms_unscaled = sqrt(mean_square);

      // 3. Normalize from 16-bit sample range to [-1.0, 1.0]
      float rms = rms_unscaled / 32768.0f;

      // Scale the final value to the display height
      waveformCache_[x] = (uint8_t)(rms * BITMAPHEIGHT);
    } else {
      waveformCache_[x] = 0;
    }
  }
  waveformCacheValid_ = true;

  // set end point variable
  endVar_.SetInt(tempSampleSize_);

  // log duration, sample count, peak vol for file
  Trace::Log("SAMPLEEDITOR", "Loaded %d frames, peak:%d from %s",
             tempSampleSize_, peakAmplitude, filename.c_str());
  fullWaveformRedraw_ = true;
}

bool SampleEditorView::goProjectSamplesDir() {
  auto fs = FileSystem::GetInstance();
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
      return false; // Abort if we can't find the project directory
    }
  } else {
    Trace::Error(
        "SampleEditorView: No project data available to find samples dir.");
    fs->chdir("/");
    return false; // Abort if project data is missing
  }
  return true;
}
