#include "record.h"

void StartMonitoring() {}
void StopMonitoring() {}
void Record(void *) {}
bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds) {
  return false;
}
void StopRecording() {}
void RequestStopRecording() {}
bool WaitForRecordingStop(uint32_t timeoutMs) { return true; }
void FinishStopRecording() {}

void SetInputSource(RecordSource source) {}

void SetLineInGain(uint8_t) {}

void SetMicGain(uint8_t) {}

bool IsRecordingActive() { return false; }
bool IsSavingRecording() { return false; }
uint8_t GetSavingProgressPercent() { return 0; }
bool DidLastRecordingCaptureAudio() { return false; }
