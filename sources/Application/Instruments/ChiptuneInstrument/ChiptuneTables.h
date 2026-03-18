/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "ChiptuneCompileTimeFunctions.h"

// precalculated semitone ratios for pitch slides (Q16.16 format)
<<<<<<< HEAD
constexpr auto semitoneRatioQ16 = makeRatioLUT(std::make_index_sequence<256>{});
// precalculated frequency table midi notes -12 to 127+12
constexpr auto frequencyLUT = makeFreqLUT(std::make_index_sequence<128 + 24>{});
// precalculated attack coefficients for envelope (0-64)
constexpr auto attackCoeffLUT = makeAttackLUT(std::make_index_sequence<65>{});
// precalculated decay coefficients for envelope (0-64)
constexpr auto decayCoeffLUT = makeDecayLUT(std::make_index_sequence<65>{});
// precalculated sine wave values for vibrato (0-63 + sentinel)
constexpr auto sine64LUT = makeSine64LUT(std::make_index_sequence<65>{});
=======
constexpr auto semitoneRatioQ16 = gen_semi_lut(std::make_index_sequence<256>{});
// precalculated frequency table midi notes -12 to 127+12
constexpr auto frequencyLUT = gen_frq_lut(std::make_index_sequence<128 + 24>{});
// precalculated attack coefficients for envelope (0-64)
constexpr auto attackCoeffLUT = gen_attack_lut(std::make_index_sequence<65>{});
// precalculated decay coefficients for envelope (0-64)
constexpr auto decayCoeffLUT = gen_decay_lut(std::make_index_sequence<65>{});
// precalculated sine wave values for vibrato (0-63 + sentinel)
constexpr auto sine64LUT = gen_sine64_lut(std::make_index_sequence<65>{});
>>>>>>> d81ac5f3 (refactoring and cleanup)
