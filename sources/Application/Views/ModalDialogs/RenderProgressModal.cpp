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
                                                 const char *message,
                                                 ProgressDisplayMode
                                                     progressDisplayMode) {
  if (inUse_) {
    auto *existing = reinterpret_cast<RenderProgressModal *>(storage_);
    existing->~RenderProgressModal();
    inUse_ = false;
  }
  inUse_ = true;
  return new (storage_)
      RenderProgressModal(view, title, message, progressDisplayMode);
}

RenderProgressModal::RenderProgressModal(View &view, const char *title,
                                         const char *message,
                                         ProgressDisplayMode progressDisplayMode)
    : ModalView(view), title_(title), message_(message), totalSamples_(0.0f),
      progressDisplayMode_(progressDisplayMode) {
  dialogWidth_ = title_.size();
  if (message_.size() > dialogWidth_) {
    dialogWidth_ = message_.size();
  }
  if (dialogWidth_ < 16u) {
    dialogWidth_ = 16u;
  }
}

RenderProgressModal::~RenderProgressModal() {}

void RenderProgressModal::Destroy() {
  this->~RenderProgressModal();
  inUse_ = false;
}

void RenderProgressModal::DrawView() {
  // Calculate window size
  const uint32_t width = getDialogWidth();
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

  // Draw action button
  SetColor(CD_NORMAL);
  y++;
  props.invert_ = true;
  // Use a fixed-width label area to avoid stale characters when label shrinks.
  x = width / 2 - 3;
  DrawString(x, y, renderComplete_ ? "  OK  " : "Cancel", props);
}

void RenderProgressModal::OnPlayerUpdate(PlayerEventType eventType,
                                         unsigned int currentTick) {
  (void)eventType;
  (void)currentTick;
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

  if (isRunning) {
    renderStarted_ = true;
    if (progressDisplayMode_ == ProgressDisplayMode::ElapsedTime) {
      totalSamples_ = player->GetPlayTime() * SAMPLE_RATE;
    } else {
      // calculate the percentage progress of the song we have rendered
      bool hasActiveRow = false;
      const int currentRow = getCurrentRenderedSongRow(&hasActiveRow);
      if (hasActiveRow) {
        if (!startSongRowCaptured_) {
          startSongRow_ = currentRow;
          startSongRowCaptured_ = true;
          initializeSongProgressTracking();
        }
        if (progressChannel_ >= 0) {
          const int renderedUnits =
              calculateChannelRenderedUnits(progressChannel_, startSongRow_);
          if (renderedUnits > renderedUnits_) {
            renderedUnits_ = renderedUnits;
          }
        }
      }
    }
    isDirty_ = true;
  } else if (renderStarted_ && !renderComplete_) {
    renderComplete_ = true;
    message_ = "Render Complete!";
    isDirty_ = true;
  }

  if (!isDirty_) {
    return;
  }
  isDirty_ = false;

  const uint32_t width = getDialogWidth();
  int32_t y = 2;
  GUITextProperties props;

  if (renderComplete_) {
    ClearTextRect(0, y - 1, width, 1);
    SetColor(CD_INFO);
    int32_t x = (width - message_.size()) / 2;
    DrawString(x, y - 1, message_.c_str(), props);
  }

  GUIPoint progressPos(width / 2 - 2, y);
  SetColor(CD_NORMAL);
  drawRenderProgress(progressPos, props);

  // Keep button label in sync with render state.
  ClearTextRect(0, y + 1, width, 1);
  props.invert_ = true;
  DrawString(width / 2 - 3, y + 1, renderComplete_ ? "  OK  " : "Cancel",
             props);
}

void RenderProgressModal::drawRenderProgress(GUIPoint &pos,
                                             GUITextProperties &props) {
  const char *spinnerchars = "|/-\\";
  char spinner = spinnerchars[spinner_++ % 4];

  char buffer[12];
  if (progressDisplayMode_ == ProgressDisplayMode::SongPercent) {
    uint8_t percent = calculateSongRenderPercent();
    sprintf(buffer, "%3d%% %c", percent, spinner);
  } else {
    // Calculate time in seconds from total samples
    uint8_t seconds = static_cast<uint8_t>(totalSamples_ / SAMPLE_RATE);
    uint8_t minutes = seconds / 60;
    seconds %= 60;
    sprintf(buffer, "%02d:%02d %c", minutes, seconds, spinner);
  }

  DrawString(pos._x, pos._y, buffer, props);
}

uint32_t RenderProgressModal::getDialogWidth() const {
  return dialogWidth_;
}

int RenderProgressModal::getCurrentRenderedSongRow(bool *hasActive) const {
  if (hasActive != nullptr) {
    *hasActive = false;
  }
  if (viewData_ == nullptr) {
    return 0;
  }

  int currentRow = 0;
  bool foundActive = false;
  for (int channel = 0; channel < SONG_CHANNEL_COUNT; channel++) {
    if (viewData_->currentPlayPhrase_[channel] == EMPTY_SONG_VALUE) {
      continue;
    }
    const int row = viewData_->songPlayPos_[channel];
    if (!foundActive || row > currentRow) {
      currentRow = row;
      foundActive = true;
    }
  }

  if (hasActive != nullptr) {
    *hasActive = foundActive;
  }

  if (currentRow < 0) {
    currentRow = 0;
  } else if (currentRow >= SONG_ROW_COUNT) {
    currentRow = SONG_ROW_COUNT - 1;
  }
  return currentRow;
}

int RenderProgressModal::getChainPhraseCount(int songRow, int channel) const {
  if (viewData_ == nullptr || viewData_->song_ == nullptr) {
    return 0;
  }
  if (songRow < 0 || songRow >= SONG_ROW_COUNT || channel < 0 ||
      channel >= SONG_CHANNEL_COUNT) {
    return 0;
  }

  const Song *song = viewData_->song_;
  const unsigned char chain =
      song->data_[songRow * SONG_CHANNEL_COUNT + channel];
  if (chain == EMPTY_SONG_VALUE) {
    return 0;
  }

  int phraseCount = 0;
  for (int i = 0; i < PHRASES_PER_CHAIN; i++) {
    if (song->chain_.data_[chain * PHRASES_PER_CHAIN + i] == EMPTY_SONG_VALUE) {
      break;
    }
    phraseCount++;
  }
  return phraseCount;
}

int RenderProgressModal::calculateChannelTotalRenderUnits(int channel,
                                                          int startSongRow) const {
  if (startSongRow < 0 || startSongRow >= SONG_ROW_COUNT) {
    return 0;
  }

  int totalUnits = 0;
  for (int row = startSongRow; row < SONG_ROW_COUNT; row++) {
    const int phraseCount = getChainPhraseCount(row, channel);
    if (phraseCount <= 0) {
      break;
    }
    totalUnits += phraseCount * STEPS_PER_PHRASE;

    if (row + 1 >= SONG_ROW_COUNT || getChainPhraseCount(row + 1, channel) <= 0) {
      break;
    }
  }

  return totalUnits;
}

int RenderProgressModal::calculateChannelRenderedUnits(int channel,
                                                       int startSongRow) const {
  if (startSongRow < 0 || startSongRow >= SONG_ROW_COUNT || viewData_ == nullptr) {
    return 0;
  }
  if (channel < 0 || channel >= SONG_CHANNEL_COUNT) {
    return 0;
  }

  int currentSongRow = viewData_->songPlayPos_[channel];
  if (currentSongRow < startSongRow) {
    return 0;
  }

  int renderedUnits = 0;
  for (int row = startSongRow; row < currentSongRow && row < SONG_ROW_COUNT; row++) {
    const int phraseCount = getChainPhraseCount(row, channel);
    if (phraseCount <= 0) {
      return renderedUnits;
    }
    renderedUnits += phraseCount * STEPS_PER_PHRASE;
  }

  if (currentSongRow >= SONG_ROW_COUNT) {
    return totalRenderUnits_;
  }

  const int currentPhraseCount = getChainPhraseCount(currentSongRow, channel);
  if (currentPhraseCount <= 0) {
    return renderedUnits;
  }

  int chainPos = viewData_->chainPlayPos_[channel];
  if (chainPos < 0) {
    chainPos = 0;
  } else if (chainPos >= currentPhraseCount) {
    chainPos = currentPhraseCount - 1;
  }

  int phrasePos = viewData_->phrasePlayPos_[channel];
  if (phrasePos < 0) {
    phrasePos = 0;
  } else if (phrasePos >= STEPS_PER_PHRASE) {
    phrasePos = STEPS_PER_PHRASE - 1;
  }

  renderedUnits += chainPos * STEPS_PER_PHRASE + phrasePos;
  if (renderedUnits > totalRenderUnits_) {
    renderedUnits = totalRenderUnits_;
  }
  return renderedUnits;
}

void RenderProgressModal::initializeSongProgressTracking() {
  progressChannel_ = -1;
  totalRenderUnits_ = 1;
  renderedUnits_ = 0;

  int bestTotalUnits = 0;
  for (int channel = 0; channel < SONG_CHANNEL_COUNT; channel++) {
    const int totalUnits = calculateChannelTotalRenderUnits(channel, startSongRow_);
    if (totalUnits <= 0) {
      continue;
    }
    if (progressChannel_ < 0 || totalUnits < bestTotalUnits) {
      progressChannel_ = channel;
      bestTotalUnits = totalUnits;
    }
  }

  if (progressChannel_ >= 0 && bestTotalUnits > 0) {
    totalRenderUnits_ = bestTotalUnits;
  }
}

int RenderProgressModal::calculateSongRenderPercent() const {
  if (renderComplete_) {
    return 100;
  }
  if (!renderStarted_ || !startSongRowCaptured_ || progressChannel_ < 0) {
    return 0;
  }

  int totalUnits = totalRenderUnits_;
  if (totalUnits <= 0) {
    totalUnits = 1;
  }

  int renderedUnits = renderedUnits_;
  if (renderedUnits < 0) {
    renderedUnits = 0;
  } else if (renderedUnits > totalUnits) {
    renderedUnits = totalUnits;
  }

  int percent = (renderedUnits * 100) / totalUnits;
  if (percent > 99) {
    percent = 99;
  }
  return percent;
}
