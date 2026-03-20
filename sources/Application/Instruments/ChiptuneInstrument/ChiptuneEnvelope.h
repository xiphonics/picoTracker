/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "ChiptuneEnums.h"
#include "ChiptuneMath.h"
#include "ChiptuneTables.h"

#pragma pack(push, 1)
typedef struct envelope_t {
  uint16_t value;
  uint16_t coefficient; // q0.16
  uint16_t attack;
  uint16_t decay;
  chiptune_env_state_e state;

  void set_attack(uint8_t a) {
    // map 8 bit attack value to 16 bit coefficient using LUT and interpolation
    attack = interpolateU16(attackCoeffLUT.data(), a);
  }

  void set_decay(uint8_t d) {
    // map 8 bit decay value to 16 bit coefficient using LUT and interpolation
    decay = interpolateU16(decayCoeffLUT.data(), d);
  }

  void trigger() {
    coefficient = attack;
    state = envAttack;
    value = 0;
  }

  void tick() {
    if (state == envIdle)
      return;

    uint32_t diff = (uint32_t)(state == envAttack ? 0xFFFF : 0) - value;
    int32_t tmp = value + ((diff * coefficient) >> 16);

    if (state == envAttack && tmp >= envAttackThreshold) {
      tmp = 0xFFFF;
      coefficient = decay;
      state = envDecay;
    } else if (tmp <= envDecayThreshold) {
      tmp = 0;
      state = envIdle;
    }

    value = tmp;
  }
} envelope_t;
#pragma pack(pop)