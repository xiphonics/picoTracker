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

#define RECORD_BUFFER_SIZE 35280 // Recording buffer is 200ms long
extern uint16_t recordBuffer[RECORD_BUFFER_SIZE];

#define LINEIN_GAIN_MINDB -6
#define LINEIN_GAIN_MAXDB 24
#define MIC_GAIN_MINDB 0  // can be negitive but mic level is already very low
#define MIC_GAIN_MAXDB 20 // per TLV320 datasheet max gain is 20dB

static StackType_t RecordStack[1024];
static StaticTask_t RecordTCB;
extern TaskHandle_t RecordHandle;

enum RecordSource { AllOff, LineIn, Mic, USBIn };

void Record(void *);
bool StartRecording(const char *filename, uint8_t threshold,

                    uint32_t milliseconds);
void StopRecording();
void RequestStopRecording();
bool WaitForRecordingStop(uint32_t timeoutMs);
void FinishStopRecording();
void StartMonitoring();
void StopMonitoring();
void SetInputSource(RecordSource source);
void SetLineInGain(uint8_t gainDb);
void SetMicGain(uint8_t gainDb);
bool IsRecordingActive();
bool IsSavingRecording();
uint8_t GetSavingProgressPercent();
// True only if the last recording captured samples and was persisted to disk.
bool DidLastRecordingCaptureAudio();

#endif
