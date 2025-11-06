/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "RenderProgressModal.h"
#include "Application/Player/Player.h"
#include "Application/Player/SyncMaster.h"
#include "Application/Views/BaseClasses/View.h"
#include "MessageBox.h"
#include "UIFramework/BasicDatas/GUIPoint.h"
#include <stdio.h>

RenderProgressModal::RenderProgressModal(View &view, const char *title,
                                         const char *message)
    : ModalView(view), title_(title), message_(message), totalSamples_(0.0f) {

  tempo_ = SyncMaster::instance().GetTempo();
}

RenderProgressModal::~RenderProgressModal() {}

void RenderProgressModal::DrawView() {
  // Calculate window size
  int width = title_.size() > message_.size() ? title_.size() : message_.size();
  width = width > 16 ? width : 16; // Minimum width for time display
  SetWindow(width, 4); // Height of 4 for title, message, time, and button

  // Draw title
  int y = 0;
  int x = (width - title_.size()) / 2;
  GUITextProperties props;
  SetColor(CD_WARN);
  DrawString(x, y, title_.c_str(), props);

  // Draw message
  y++;
  x = (width - message_.size()) / 2;
  DrawString(x, y, message_.c_str(), props);

  // Draw render progress
  y++;
  GUIPoint progressPos(width / 2 - 2, y); // Center the progress display
  drawRenderProgress(progressPos, props);

  // Draw OK button
  SetColor(CD_NORMAL);
  y++;
  props.invert_ = true;
  x = width / 2 - 1; // Center the OK button
  DrawString(x, y, "OK", props);
}

void RenderProgressModal::OnPlayerUpdate(PlayerEventType eventType,
                                         unsigned int currentTick) {
  // This runs on core1 (audio thread)
  auto &player = Player::instance();

  // Check if player has stopped and we haven't marked as complete yet
  if (!player.IsRunning() && !renderComplete_) {
    renderComplete_ = true;
    message_ = "Render Complete!"; // Update the message
    isDirty_ = true;               // Mark view as dirty to trigger redraw
  }
  // Only update progress if we're still rendering
  else if (player.IsRunning()) {
    // Calculate samples for this buffer based on the tempo
    float samplesThisBuffer = calculateSamplesPerBuffer(tempo_);
    // Add to our total sample count and mark as dirty for redraw
    totalSamples_ += samplesThisBuffer;
    isDirty_ = true;
  }
}

void RenderProgressModal::OnFocus() {}

void RenderProgressModal::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (mask & EPBM_ENTER && pressed) {
    // If player is still running, stop it first
    if (Player::instance().IsRunning()) {
      Player::instance().Stop();
    }
    // Always allow closing the modal, whether rendering is complete or not
    EndModal(MBL_OK);
    return; // Return early to prevent setting dirty flag unnecessarily
  }
  // Only set dirty if we didn't handle the button press
  isDirty_ = true;
}

void RenderProgressModal::AnimationUpdate() {
  // This runs on core0 (UI thread)
  if (isDirty_) {
    isDirty_ = false;

    int width =
        title_.size() > message_.size() ? title_.size() : message_.size();
    width = width > 16 ? width : 16; // Minimum width for time display
    int y = 2;
    GUITextProperties props;

    // Update completion message if needed
    if (renderComplete_) {
      SetColor(CD_INFO);
      message_ = "Render Complete!";
      int x = (width - message_.size()) / 2;
      DrawString(x, y - 1, message_.c_str(), props);
    }

    // Always redraw the progress display
    GUIPoint progressPos(width / 2 - 2, y);
    SetColor(CD_NORMAL);
    drawRenderProgress(progressPos, props);
  }
}

void RenderProgressModal::drawRenderProgress(GUIPoint &pos,
                                             GUITextProperties &props) {
  // Calculate time in seconds from total samples
  int seconds = static_cast<int>(totalSamples_ / SAMPLE_RATE);
  int minutes = seconds / 60;
  seconds %= 60;

  // Format as MM:SS
  char buffer[10];
  const char *spinnerchars = "|/-\\";
  char spinner = spinnerchars[spinner_++ % 4];
  sprintf(buffer, "%02d:%02d %c", minutes, seconds, spinner);
  DrawString(pos._x, pos._y, buffer, props);
}

float RenderProgressModal::calculateSamplesPerBuffer(int tempo) {
  // Calculate samples per buffer using the same formula as in
  // SyncMaster::SetTempo playSampleCount_ = 60.0f * driverRate * 2.0f / tempo_
  // / 8.0f / float(AUDIO_SLICES_PER_STEP);

  float samplesPerBuffer = 60.0f * SAMPLE_RATE * 2.0f / tempo / 8.0f /
                           static_cast<float>(AUDIO_SLICES_PER_STEP);
  return samplesPerBuffer;
}
