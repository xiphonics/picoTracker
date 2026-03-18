/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

// Generator functions for compile-time lookup tables used in ChiptuneEngine.h
// Not (to be) used at runtime.

// Tables are precalculated rather than embedded to allow tweaking the
// parameters of the functions (e.g. envelope curve) on the fly and to avoid
// having to recalculate the table if the sample rate changes
#include <array>
#include <cmath>
#include <cstdint>

static constexpr float kSampleRate = 44100.0;

// power function
consteval double pow_ct(double base, double exp) {
  if (exp == 0.0)
    return 1.0;
  if (exp < 0.0)
    return 1.0 / pow_ct(base, -exp);

  double result = 1.0;
  int int_exp = static_cast<int>(exp);
  double frac_exp = exp - int_exp;

  // Integer part
  for (int i = 0; i < int_exp; i++) {
    result *= base;
  }

  // Fractional part approximation using Taylor series for 2^x
  if (frac_exp != 0.0) {
    double x = frac_exp * 0.693147180559945309417; // ln2
    double term = 1.0;
    double sum = 1.0;
    for (int i = 1; i < 20; i++) {
      sum += (term *= x / i);
    }
    result *= sum;
  }
  return result;
}

// Compile-time sine function using Taylor series
consteval double sin_ct(double x) {
  // Normalize x to [-pi, pi]
  constexpr double PI = 3.141592653589793;
  while (x > PI)
    x -= 2 * PI;
  while (x < -PI)
    x += 2 * PI;

  // Taylor series: sin(x) = x - x^3/3! + x^5/5! - x^7/7! + ...
  double result = 0.0;
  double term = x;
  double x_squared = x * x;

  for (int n = 1; n <= 15; n += 2) {
    result += term;
    term *= -x_squared / ((n + 1) * (n + 2));
  }

  return result;
}

// Calculate semitone ratio in Q16.16 format at compile time
consteval uint32_t calculateSemitoneRatioQ16(int index) {
  int semitones = index - 128; // -128 to +127
  double ratio = pow_ct(2.0, semitones / 12.0);
  return static_cast<uint32_t>(ratio * 65536.0 + 0.5);
}

// Generate the lookup table at compile time
template <size_t... Is>
consteval auto gen_semi_lut(std::index_sequence<Is...>) {
  return std::array<uint32_t, sizeof...(Is)>{
      calculate_semitone_ratio_q16(Is)...};
}

// Calculate MIDI note frequency at compile time
// Formula:
// freq = 440 * 2^((note-69)/12) * (kSampleRate/440) = 100 * 2^((note-69)/12)
// * 44100/100
// The values appear to be phase increments for sampling rate phase_increment =
// frequency * 2^32 / kSampleRate
consteval int32_t calculateFrequency(int midiNote) {
  // MIDI note to frequency: f = 440 * 2^((note-69)/12)
  // Phase increment = f * 2^32 / kSampleRate
  double semitones = (midiNote - 69.0) / 12.0;
  double frequency = 440.0 * pow_ct(2.0, semitones);
  double phaseIncrement = frequency * 4294967296.0 / kSampleRate;

  // Clamp to int32 range to prevent overflow
  if (phaseIncrement > 2147483647.0)
    phaseIncrement = 2147483647.0;
  if (phaseIncrement < -2147483648.0)
    phaseIncrement = -2147483648.0;

  return static_cast<int32_t>(phaseIncrement + 0.5);
}

template <size_t... Is> consteval auto gen_frq_lut(std::index_sequence<Is...>) {
  return std::array<int32_t, sizeof...(Is)>{
      calculateFrequency(static_cast<int>(Is) - 12)...};
}

// Calculate attack/decay coefficients at compile time
// These coefficients are used in exponential envelope interpolation
// The formula is: coeff = 65535 * exp(-k * index)
// attack goes from 65535 down to 326
// decay also follows a similar exponential curve
consteval uint16_t calculateAttackCoeff(int index) {
  if (index >= 64)
    return 326;

  // Exponential decay from 65535 to ~326 over 64 steps
  // Using empirical fit: coeff ≈ 65535 * exp(-0.08 * index)
  double k = 0.0825; // decay constant
  double coeff = 65535.0 * pow_ct(2.718281828459045, -k * index);

  // Clamp to minimum
  if (coeff < 326.0)
    coeff = 326.0;

  return static_cast<uint16_t>(coeff + 0.5);
}

consteval uint16_t calculateDecayCoeff(int index) {
  if (index >= 64)
    return 130;

  // Exponential decay from 65535 to ~130 over 64 steps
  // Using empirical fit: coeff ≈ 65535 * exp(-0.1 * index)
  double k = 0.102; // decay constant
  double coeff = 65535.0 * pow_ct(2.718281828459045, -k * index);

  // Clamp to minimum
  if (coeff < 130.0)
    coeff = 130.0;

  return static_cast<uint16_t>(coeff + 0.5);
}

template <size_t... Is>
consteval auto gen_attack_lut(std::index_sequence<Is...>) {
  return std::array<uint16_t, sizeof...(Is)>{calculate_attack_coeff(Is)...};
}

template <size_t... Is>
consteval auto gen_decay_lut(std::index_sequence<Is...>) {
  return std::array<uint16_t, sizeof...(Is)>{calculate_decay_coeff(Is)...};
}

// Calculate sine wave value at compile time
// Generates a sine wave with 64 steps (0-63) plus one sentinel at index 64
consteval int8_t calculateSine64LUT(int index) {
  if (index >= 64)
    return 0; // sentinel value

  constexpr double PI = 3.141592653589793;
  double angle = (index * 2.0 * PI) / 64.0;
  double sine = sin_ct(angle);

  // Scale to -127..127 range
  int8_t result = static_cast<int8_t>(sine * 127.0 + 0.5);
  return result;
}

template <size_t... Is>
consteval auto gen_sine64_lut(std::index_sequence<Is...>) {
  return std::array<int8_t, sizeof...(Is)>{calculate_sine64_lut(Is)...};
}
