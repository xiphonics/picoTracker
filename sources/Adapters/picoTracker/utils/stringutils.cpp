#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// The result of the function must be freed
// ref: https://stackoverflow.com/a/77278639/85472
char *humanMemorySize(uint32_t bytes) {
  char *result = (char *)malloc(sizeof(char) * 20);
  const char *const sizeNames[] = {"B", "KB", "MB", "GB"};

  uint32_t i = (uint32_t)floor(log(bytes) / log(1024));
  double humanSize = bytes / pow(1024, i);
  snprintf(result, sizeof(char) * 20, "%g %s", humanSize, sizeNames[i]);

  return result;
}