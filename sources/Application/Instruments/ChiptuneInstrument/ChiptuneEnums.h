/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

enum gbConstants { gbNumWaveforms = 8 };

enum gbWaveType {
  gbWavePulse12_5,
  gbWavePulse25,
  gbWavePulse50,
  gbWaveTriangle,
  gbWaveNoiseGameBoy7,
  gbWaveNoiseNES,
  gbWaveNoiseSN76489,
  gbWaveNoiseWhite,
  gbWaveNone
};

enum gbEnvState { gbEnvIdle, gbEnvAttack, gbEnvDecay };

typedef union gbFlags {
  struct {
    uint8_t arpeggio : 1;
    uint8_t legato : 1;
    uint8_t retrigger : 1;
    uint8_t volume : 1;
    uint8_t burst_end : 1;
    uint8_t unused : 2;
  };
  uint8_t byte;
} gbFlags;