/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _RENDER_PROGRESS_MODAL_H_
#define _RENDER_PROGRESS_MODAL_H_

#include "Application/Player/Player.h"
#include "Application/Views/BaseClasses/ModalView.h"
#include <cstdint>
#include <etl/string.h>

// Forward declarations
class GUIPoint;
class GUITextProperties;

// Progress message box with render progress display
class RenderProgressModal : public ModalView {
public:
  enum class ProgressDisplayMode { ElapsedTime, SongPercent };

  static RenderProgressModal *Create(View &view, const char *title,
                                     const char *message,
                                     ProgressDisplayMode progressDisplayMode =
                                         ProgressDisplayMode::ElapsedTime);

  // Constructor taking a view, title and message
  RenderProgressModal(View &view, const char *title, const char *message,
                      ProgressDisplayMode progressDisplayMode);

  // Virtual destructor
  virtual ~RenderProgressModal();
  virtual void Destroy() override;

  // ModalView overrides
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick);
  virtual void OnFocus();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void AnimationUpdate();

private:
  RenderProgressModal(const RenderProgressModal &) = delete;
  RenderProgressModal &operator=(const RenderProgressModal &) = delete;

  // Helper method to draw the render progress
  void drawRenderProgress(GUIPoint &pos, GUITextProperties &props);
  uint32_t getDialogWidth() const;
  int calculateSongRenderPercent() const;
  int getCurrentRenderedSongRow(bool *hasActive = nullptr) const;
  int getChainPhraseCount(int songRow, int channel) const;
  int calculateChannelTotalRenderUnits(int channel, int startSongRow) const;
  int calculateChannelRenderedUnits(int channel, int startSongRow) const;
  void initializeSongProgressTracking();

  // Title and message strings
  etl::string<20> title_;
  etl::string<32> message_;

  // Track total rendered samples (calculated from player updates)
  float totalSamples_;
  bool renderComplete_ = false;
  bool renderStarted_ = false;

  ProgressDisplayMode progressDisplayMode_;
  int startSongRow_ = 0;
  int renderedUnits_ = 0;
  int totalRenderUnits_ = 1;
  int progressChannel_ = -1;
  bool startSongRowCaptured_ = false;

  unsigned char spinner_ = 0;

  // Constants for sample rate calculations
  static const int SAMPLE_RATE = 44100;

  static bool inUse_;
  static void *storage_;
};

#endif // _RENDER_PROGRESS_MODAL_H_
