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

enum RecordSource { AllOff, LineIn, Mic, USBIn };

void Record(void *);
bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds);
void StopRecording();
void StartMonitoring();
void StopMonitoring();
void SetInputSource(RecordSource source);

#endif
