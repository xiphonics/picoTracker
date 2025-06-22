/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

// Helper function to convert linear amplitude to dB
inline int amplitudeToDb(int amplitude) {
  if (amplitude <= 0) {
    return -100; // Or some other minimum value for silence
  }
  return static_cast<int>(20.0 *
                          log10(static_cast<double>(amplitude) / 32767.0));
}