/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _RECORD_H_
#define _RECORD_H_

#include <cstdint>

// DUMMY values as no linein or mic on pico
static constexpr int lineInGainMinDb = 0;
static constexpr int lineInGainMaxDb = 0;
static constexpr int micGainMinDb = 0;
static constexpr int micGainMaxDb = 0;

enum RecordSource { AllOff, LineIn, Mic, USBIn };

void Record(void *);
bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds);
void StopRecording();
void StartMonitoring();
void StopMonitoring();
void SetInputSource(RecordSource source);
void SetLineInGain(int gainDb);
void SetMicGain(int gainDb);
bool IsRecordingActive();

#endif
