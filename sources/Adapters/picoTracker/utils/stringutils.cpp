#include "pico/stdlib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// ref: https://stackoverflow.com/a/77278639/85472
void humanMemorySize(uint32_t bytes, char *output) {
  const char *const sizeNames[] = {"B", "KB", "MB", "GB"};

  uint32_t i = (uint32_t)floor(log(bytes) / log(1024));
  float humanSize = bytes / pow(1024, i);
  snprintf(output, sizeof(char) * 12, "%.1f%s", humanSize, sizeNames[i]);
}

// Pass in a 10 char buffer or 16 char if using showPercent
// ref: https://stackoverflow.com/a/36315819/85472
#define PBSTR "||||||"
#define PBWIDTH 6
void printProgress(float percentage, char *buffer, bool showPercent) {
  int val = (int)(percentage * 100);
  int lpad = (int)(percentage * PBWIDTH);
  int rpad = PBWIDTH - lpad;
  if (showPercent) {
    sprintf(buffer, "\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
  } else {
    sprintf(buffer, "[%.*s%*s]\n", lpad, PBSTR, rpad, "");
  }
}
