#include "record.h"

void StartMonitoring() {}
void StopMonitoring() {}
void Record(void *) {}
bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds) {
  return false;
}
void StopRecording() {}