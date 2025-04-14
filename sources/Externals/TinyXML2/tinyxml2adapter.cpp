#include "tinyxml2adapter.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"
#include <stdarg.h>
#include <string.h>

void fprintf(I_File *f, const char *fmt, ...) {
  char buffer[256];
  va_list args;
  va_start(args, fmt);

  vsprintf(buffer, fmt, args);
  int len = strlen(buffer);
  f->Write(buffer, 1, len);
};
