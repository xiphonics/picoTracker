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
#include "UIFramework/BasicDatas/GUIPoint.h"
#include <cstdint>
#include <new>
#include <stdio.h>

bool RenderProgressModal::inUse_ = false;
alignas(RenderProgressModal) static unsigned char RenderProgressModalStorage
    [sizeof(RenderProgressModal)];
void *RenderProgressModal::storage_ = RenderProgressModalStorage;

RenderProgressModal *RenderProgressModal::Create(View &view, const char *title,
                                                 const char *message) {
  if (inUse_) {
    auto *existing = reinterpret_cast<RenderProgressModal *>(storage_);
    existing->~RenderProgressModal();
    inUse_ = false;
  }
  inUse_ = true;
  return new (storage_) RenderProgressModal(view, title, message);
}

RenderProgressModal::RenderProgressModal(View &view, const char *title,
                                         const char *message)
    : ModalView(view), title_(title), message_(message), totalSamples_(0.0f) {
  tempo_ = SyncMaster::GetInstance()->GetTempo();
}

RenderProgressModal::~RenderProgressModal() {}

void RenderProgressModal::Destroy() {
  this->~RenderProgressModal();
  inUse_ = false;
}

void RenderProgressModal::DrawView() {
  // Calculate window size
  uint32_t width = title_.size();
  if (message_.size() > width) {
    width = message_.size();
  }
  width = width > 16u ? width : 16u; // Minimum width for time display
  SetWindow(width, 4); // Height of 4 for title, message, time, and button

  // Draw title
  int32_t y = 0;
  int32_t x = (width - title_.size()) / 2;
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
  Player *player = Player::GetInstance();
  const bool isRunning = player && player->IsRunning();
  const bool hasJustFinished = renderStarted_ && !isRunning;

  // Only mark completion if we have observed an active render first.
  if (isRunning) {
    renderStarted_ = true;
  }

  // Mark completion after transitioning from started -> stopped.
  if (hasJustFinished && !renderComplete_) {
    renderComplete_ = true;
    message_ = "Render Complete!"; // Update the message
    isDirty_ = true;               // Mark view as dirty to trigger redraw
  }
  // Only update progress if we're still rendering
  else if (isRunning) {
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
    Player *player = Player::GetInstance();
    if (player && player->IsRunning()) {
      player->Stop();
    }
    // Always allow closing the modal, whether rendering is complete or not
    EndModal(0);
    return; // Return early to prevent setting dirty flag unnecessarily
  }
  // Only set dirty if we didn't handle the button press
  isDirty_ = true;
}

void RenderProgressModal::AnimationUpdate() {
  // This runs on core0 (UI thread). Keep updating progress every UI tick
  // while rendering so the dialog stays responsive even if PET_UPDATE events
  // are sparse during stems.
  Player *player = Player::GetInstance();
  const bool isRunning = player && player->IsRunning();
  const bool hasJustFinished = renderStarted_ && !isRunning;

  if (isRunning) {
    renderStarted_ = true;
    totalSamples_ = player->GetPlayTime() * SAMPLE_RATE;
    isDirty_ = true;
  } else if (hasJustFinished && !renderComplete_) {
    renderComplete_ = true;
    message_ = "Render Complete!";
    isDirty_ = true;
  }

  if (!isDirty_) {
    return;
  }
  isDirty_ = false;

  uint32_t width = title_.size();
  if (message_.size() > width) {
    width = message_.size();
  }
  width = width > 16u ? width : 16u; // Minimum width for time display
  int32_t y = 2;
  GUITextProperties props;

  if (renderComplete_) {
    SetColor(CD_INFO);
    int32_t x = (width - message_.size()) / 2;
    DrawString(x, y - 1, message_.c_str(), props);
  }

  GUIPoint progressPos(width / 2 - 2, y);
  SetColor(CD_NORMAL);
  drawRenderProgress(progressPos, props);
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
