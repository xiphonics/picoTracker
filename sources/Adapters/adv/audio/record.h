/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _RECORD_H_
#define _RECORD_H_

#include "FreeRTOS.h"
#include "semphr.h"

#define MAX_INT32 0xFFFFFFFF

#define RECORD_BUFFER_SIZE                                                     \
  35280 // Resolution is 200ms, we want buffer to be relatively big for disk
        // writing efficiency
extern uint16_t recordBuffer[RECORD_BUFFER_SIZE];

static StackType_t RecordStack[1024];
static StaticTask_t RecordTCB;
extern TaskHandle_t RecordHandle;

enum RecordSource { AllOff, LineIn, Mic, USBIn };

void Record(void *);
bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds);
void StopRecording();
void StartMonitoring();
void StopMonitoring();
void SetInputSource(RecordSource source);

#endif
