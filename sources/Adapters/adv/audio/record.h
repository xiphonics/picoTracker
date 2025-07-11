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

#define RECORD_BUFFER_SIZE 32768 // Adjust based on sample rate and latency
extern uint8_t bufferA[RECORD_BUFFER_SIZE];
extern uint8_t bufferB[RECORD_BUFFER_SIZE];

// extern SemaphoreHandle_t record_semaphore;
// extern StaticSemaphore_t record_semaphoreBuffer;

static StackType_t RecordStack[1024];
static StaticTask_t RecordTCB;
extern TaskHandle_t RecordHandle;

void Record(void *);

#endif
