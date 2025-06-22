/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ApplicationCommandDispatcher.h"
#include "Application/Player/Player.h"

ApplicationCommandDispatcher::ApplicationCommandDispatcher() { project_ = 0; };

ApplicationCommandDispatcher::~ApplicationCommandDispatcher(){};

void ApplicationCommandDispatcher::Execute(FourCC id, float value) {
  switch (id) {
  case FourCC::TrigTempoTap:
    if (value > 0.5)
      OnTempoTap();
    break;
  case FourCC::TrigSeqQueueRow:
    if (value > 0.5)
      OnQueueRow();
    break;
  }
};

void ApplicationCommandDispatcher::Init(Project *project) {
  project_ = project;
};

void ApplicationCommandDispatcher::Close() { project_ = 0; };

void ApplicationCommandDispatcher::OnTempoTap() {
  if (!project_)
    return;
  project_->OnTempoTap();
};

void ApplicationCommandDispatcher::OnQueueRow() {
  if (!project_)
    return;
  Player *player = Player::GetInstance();
  player->SetSequencerMode(SM_LIVE);
  player->OnSongStartButton(0, 7, false, false);
};

#define TEMPO_NUDGE 3
void ApplicationCommandDispatcher::OnNudgeDown() {
  project_->NudgeTempo(-TEMPO_NUDGE);
};

void ApplicationCommandDispatcher::OnNudgeUp() {
  project_->NudgeTempo(TEMPO_NUDGE);
};
