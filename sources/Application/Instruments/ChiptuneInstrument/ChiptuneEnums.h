/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

<<<<<<< HEAD
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
=======
#include "Foundation/Constants/SpecialCharacters.h"

enum chiptune_constants_e {
  envAttackThreshold = 65530,
  envDecayThreshold = 10,
  numWaveforms = 8,
  q16_16_1 = 0x0001'0000,
  ticks100Hz = 441,
  ticks1000Hz = 44,
  vibratoFrequency = 0xFFF,
};

enum chiptune_wave_type_e : uint8_t {
  wavePulse12_5 = 0,
  wavePulse25,
  wavePulse50,
  waveTriangle,
  waveNoiseGameBoy7,
  waveNoiseNES,
  waveNoiseSN76489,
  waveNoiseWhite,
  waveNone
};

enum chiptune_env_state_e : uint8_t { envIdle, envAttack, envDecay };

typedef struct ui_config_t {
  const char *format;
  int min;
  int max;
  int step;
  int bigStep;
} ui_config_t;

#define horz_line char_border_single_horizontal_s
#define top_indent " " char_border_single_verticalRight_s horz_line
#define bottom_indent " " char_border_single_bottomLeft_s horz_line
static struct chiptune_instrument_ui_t {
  ui_config_t wave = {"Waveform:   %s", 0, numWaveforms - 1, 1, 1};
  ui_config_t transpose = {"Transpose: %+03d", -24, 24, 1, 12};
  ui_config_t level = {"Level:      %02X", 0, 255, 1, 16};
  ui_config_t burst = {"Burst:      %02X", 1, 255, 1, 16};
  ui_config_t arp = {"Arp Speed:  %02X", 1, 32, 1, 8};
  ui_config_t length = {"Length:     %02X", 1, 255, 1, 16};
  ui_config_t table = {"Table:      %02X", 0, TABLE_COUNT - 1, 1, 0x10};
  ui_config_t attack = {"Attack:     %02X", 0, 255, 1, 16};
  ui_config_t decay = {"Decay:      %02X", 0, 255, 1, 16};
  ui_config_t vibrato_amount = {top_indent " Amount: %02X", 0, 255, 1, 16};
  ui_config_t vibrato_delay = {bottom_indent " Delay:  %02X", 0, 255, 1, 16};
  ui_config_t sweep_time = {top_indent " Length: %02X", 0, 255, 1, 16};
  ui_config_t sweep_amount = {bottom_indent " Amount:%+03d" - 127, 127, 1, 16};
} chiptune_instrument_ui_t;

enum chiptune_instrument_defaults_e {
  defaultArpSpeed = 0x12,
  defaultAttack = 0x00,
  defaultBurst = -1,
  defaultDecay = 0x80,
  defaultLength = -1,
  defaultLevel = 0x80,
  defaultSweepAmount = 0x00,
  defaultSweepTime = 0x00,
  defaultTable = -1,
  defaultTranspose = 0,
  defaultVibratoDelay = 0x40,
  defaultVibratoDepth = 0x07,
  defaultWaveform = wavePulse25
>>>>>>> d81ac5f3 (refactoring and cleanup)
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