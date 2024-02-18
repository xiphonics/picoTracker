#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// The result of the function must be freed
// ref: https://stackoverflow.com/a/77278639/85472
void humanMemorySize(uint32_t bytes, char *output) {
  const char *const sizeNames[] = {"B", "KB", "MB", "GB"};

  uint32_t i = (uint32_t)floor(log(bytes) / log(1024));
  float humanSize = bytes / pow(1024, i);
  snprintf(output, sizeof(char) * 12, "%.1f%s", humanSize, sizeNames[i]);
}