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
    : FieldView(w, data), forceRedraw_(false), isPlaying_(false),
      isSingleCycle_(false), playKeyHeld_(false), waveformCacheValid_(false),
      playbackPosition_(0.0f), playbackStartFrame_(0), lastAnimationTime_(0),
      sys_(System::GetInstance()), startVar_(FourCC::VarSampleEditStart, 0),
      endVar_(FourCC::VarSampleEditEnd, 0),
      // bit of a hack to use InstrumentName but we never actually persist this
      // in any config file here
      filenameVar_(FourCC::InstrumentName, "") {
  // Initialize cached sample parameters
  start_ = 0;
  end_ = 0;
  // Initialize waveform cache to zero
  memset(waveformCache_, 0, BITMAPWIDTH * sizeof(uint8_t));
  // Clear the buffer
  memset(bitmapBuffer_, 0, BITMAPBUFFERSIZE * sizeof(uint8_t));
}

SampleEditorView::~SampleEditorView() {}

void SampleEditorView::OnFocus() {
  const auto newSampleFile = viewData_->sampleEditorFilename;
  // Load the sample using this filename and will fill in the wave data cache
  loadSample(newSampleFile);

  // Update cached sample parameters
  updateSampleParameters();

  // Force redraw of waveform
  forceRedraw_ = true;

  GUIPoint position(0, 3);
  waveformField_.emplace_back(position, BITMAPWIDTH, BITMAPHEIGHT,
                              bitmapBuffer_, 0xFFFF, 0x0000);

  // make sure we do initial draw of the waveform into bitmap for display
  updateWaveformDisplay();

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
  // no need to clear staticField_ and waveform as they are not added to
  // fieldList_

  GUIPoint position = GetAnchor();

  position._y = 12; // offset enough for bitmap field
  position._x = 5;

  auto label =
      etl::make_string_with_capacity<MAX_UITEXTFIELD_LABEL_LENGTH>("name: ");

  // Use an empty default name - we don't want to populate with sample
  // filename The display name will still be shown on the phrase screen via
  // GetDisplayName()
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> defaultName;

  nameTextField_.emplace_back(filenameVar_, position, label,
                              FourCC::InstrumentName, defaultName);
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
      forceRedraw_ = true;

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
      // Force redraw to show the playhead
      forceRedraw_ = true;
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
  if (isPlaying_) {
    // Check if the Player is still playing the sample
    if (!Player::GetInstance()->IsPlaying()) {
      // Playback has stopped (reached the end of non-looping sample)
      isPlaying_ = false;
      playbackPosition_ = 0;
      forceRedraw_ = true;
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
        isPlaying_ = false;
        Trace::Debug("PLayback stopped at end!");
      }

      // Update position (normalized to full sample range)
      playbackPosition_ = samplePos / tempSampleSize_;
      Trace::Debug("pos:%d", playbackPosition_);
      forceRedraw_ = true;
    }
  }

  // Check if we need to update the waveform display
  if (forceRedraw_) {
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

void SampleEditorView::updateWaveformDisplay() {
  Profiler p("updateWaveformDisplay");
  BitmapGraphics *gfx = BitmapGraphics::GetInstance();

#ifdef ADV
  const int scale = 2;
#else
  const int scale = 1;
#endif

  memset(bitmapBuffer_, 0, BITMAPBUFFERSIZE * sizeof(uint8_t));

  // Draw a border around the waveform display
#ifdef ADV
  // Draw rectangle using direct buffer access for 8bpp
  memset(bitmapBuffer_, 1, BITMAPWIDTH); // Top edge
  memset(bitmapBuffer_ + (BITMAPHEIGHT - 1) * BITMAPWIDTH, 1,
         BITMAPWIDTH); // Bottom edge
  for (int y = 1; y < BITMAPHEIGHT - 1; y++) {
    bitmapBuffer_[y * BITMAPWIDTH] = 1;                     // Left edge
    bitmapBuffer_[(y * BITMAPWIDTH) + BITMAPWIDTH - 1] = 1; // Right edge
  }
#else
  gfx->drawRect(buffer, scaled_width, scaled_height, 0, 0, scaled_width - 1,
                scaled_height - 1, false, true);
#endif

  // Draw markers directly to the final buffer
  {
    Profiler p("updateWaveformDisplay: draw markers");
    int fullSampleSize = tempSampleSize_;
    int startX =
        (1 + (int)(((float)start_ / fullSampleSize) * (BITMAPWIDTH - 2))) *
        scale;
#ifdef ADV
    for (int y = 1; y < BITMAPHEIGHT - 2; y++) {
      bitmapBuffer_[y * BITMAPWIDTH + startX] = 1;
    }
#else
    gfx->drawLine(buffer, scaled_width, scaled_height, startX, 1, startX,
                  scaled_height - 2, true);
#endif

    int endX =
        (1 + (int)(((float)end_ / fullSampleSize) * (BITMAPWIDTH - 2))) * scale;
#ifdef ADV
    for (int y = 1; y < BITMAPHEIGHT - 2; y++) {
      bitmapBuffer_[y * BITMAPWIDTH + endX] = 1;
    }
#else
    gfx->drawLine(buffer, scaled_width, scaled_height, endX, 1, endX,
                  scaled_height - 2, true);
#endif
  }

  // =====

  auto field = waveformField_.front();
  // Update the bitmap field and request redraw
  field.SetBitmap(bitmapBuffer_);

  SetDirty(true);

  Profiler p1("updateWaveformDisplay: REDRAW");
  field.Draw(w_);
}

short SampleEditorView::chunkBuffer_[512 * 2];

// Load sample from the current projects samples subdir
// ONLY filename for samples subidr (not full path!!) is supported for now
void SampleEditorView::loadSample(
    const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> filename) {

  // Reset temporary sample state
  tempSampleSize_ = 0;
  waveformCacheValid_ = false;
  static float sumSquares[WAVEFORM_CACHE_SIZE];
  memset(sumSquares, 0, sizeof(sumSquares));

  if (filename.empty()) {
    Trace::Error("missing sample filename");
    return;
  }

  // First, navigate to the root projects samples subdir directory
  if (!goProjectSamplesDir()) {
    Trace::Error("couldnt change to project samples dir!");
    return;
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
        // Normalize the sample to a -1.0 to 1.0 range
        float normalizedSample = sampleValue / 32768.0f;
        // Add the square of the sample to the accumulator for this cache index
        sumSquares[cacheIndex] += normalizedSample * normalizedSample;
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

  for (int x = 0; x < WAVEFORM_CACHE_SIZE; ++x) {
    if (samplesPerPixel >= 1) {
      // Calculate the Root Mean Square (RMS)
      float rms = sqrt(sumSquares[x] / samplesPerPixel);

      // Apply a scaling factor to make quiet waveforms more visible
      float scaledRms = std::min(1.0f, rms * scalingFactor);

      // Scale the final value to the display height
      waveformCache_[x] = (uint8_t)(scaledRms * BITMAPHEIGHT);
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
  forceRedraw_ = true;
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
