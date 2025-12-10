/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include <cstddef>

// Controller / channel naming
constexpr std::size_t STRING_CONTROL_NAME_MAX = 64;
constexpr std::size_t STRING_CONTROL_PATH_MAX = 256;
constexpr std::size_t STRING_CONTROLLER_CLASS_MAX = 32;
constexpr std::size_t STRING_CHANNEL_NAME_MAX = 32;

// Audio naming and paths
constexpr std::size_t STRING_AUDIO_API_MAX = 32;
constexpr std::size_t STRING_AUDIO_DEVICE_MAX = 128;
constexpr std::size_t STRING_AUDIO_RENDER_PATH_MAX = 256;

// MIDI naming
constexpr std::size_t STRING_MIDI_OUT_NAME_MAX = 64;
// MIDI input channel path fragments like "0:cc:127"
constexpr std::size_t STRING_MIDI_IN_KEY_MAX = 24;

// UI / input mappings
constexpr std::size_t STRING_EVENT_MAPPING_MAX = 32;

// Error reporting
constexpr std::size_t STRING_RESULT_MAX = 256;
