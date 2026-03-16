/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

enum chiptuneConstants {
  chiptuneNumWaveforms = 8,
  chiptuneEnvAttackThreshold = 65530,
  chiptuneEnvDecayThreshold = 10,
  chiptune100HzTicks = 441,
  chiptune1_0_q16_16 = 0x0001'0000,
  chiptuneVibratoFrequency = 0xFFF,
};

enum chiptuneWaveType : uint8_t {
  chiptuneWavePulse12_5,
  chiptuneWavePulse25,
  chiptuneWavePulse50,
  chiptuneWaveTriangle,
  chiptuneWaveNoiseGameBoy7,
  chiptuneWaveNoiseNES,
  chiptuneWaveNoiseSN76489,
  chiptuneWaveNoiseWhite,
  chiptuneWaveNone
};

enum chiptuneEnvState : uint8_t {
  chiptuneEnvIdle,
  chiptuneEnvAttack,
  chiptuneEnvDecay
};

typedef union chiptuneFlags {
  struct {
    uint8_t arpeggio : 1;
    uint8_t legato : 1;
    uint8_t retrigger : 1;
    uint8_t volume : 1;
    uint8_t burst_end : 1;
    uint8_t unused : 3;
  };
  uint8_t byte;
} chiptuneFlags;