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

  Player *player = Player::GetInstance();
  tempo_ = SyncMaster::GetInstance()->GetTempo();
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

void RenderProgressModal::OnPlayerUpdate(PlayerEventType,
                                         unsigned int currentTick) {
  int width = title_.size() > message_.size() ? title_.size() : message_.size();
  width = width > 16 ? width : 16; // Minimum width for time display
  int y = 2;
  GUITextProperties props;
  SetColor(CD_WARN);
  // Song has looped, end the rendering
  Player *player = Player::GetInstance();
  if (!player->IsRunning()) {
    // Show completion message
    SetColor(CD_INFO);
    message_ = "Render Complete!";
    int x = (width - message_.size()) / 2;
    DrawString(x, y - 1, message_.c_str(), props);
  }

  // Each time OnPlayerUpdate is called, a buffer of audio has been processed
  // Get the current tempo to calculate the actual number of samples in this
  // buffer
  // Calculate samples for this buffer based on the tempo
  float samplesThisBuffer = calculateSamplesPerBuffer(tempo_);

  // Add to our total sample count
  totalSamples_ += samplesThisBuffer;

  // Redraw just the render progress display
  GUIPoint progressPos(width / 2 - 2, y); // Center the progress display
  SetColor(CD_NORMAL);
  drawRenderProgress(progressPos, props);
}

void RenderProgressModal::OnFocus() {}

void RenderProgressModal::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (mask & EPBM_ENTER && pressed) {
    // Stop the player when OK is pressed
    Player *player = Player::GetInstance();
    if (player && player->IsRunning()) {
      player->Stop();
    }
    EndModal(MBL_OK);
  }
  isDirty_ = true;
}

void RenderProgressModal::AnimationUpdate() {
  // Mark as dirty to update the render progress display
  isDirty_ = true;
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