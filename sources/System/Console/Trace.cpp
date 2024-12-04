#include "Trace.h"
#include "Adapters/picoTracker/platform/platform.h"
#include "hardware/uart.h"
#include <string.h>

// be explicit about the nanoprintf configuration
#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#include "nanoprintf.h"

void pt_uart_putc(int c, void *context) {
  uint8_t byte = (uint8_t)c;
  // write directly to the UART instead of put_char() to save on extra function
  // calls
  uart_putc(DEBUG_UART, byte);
}

Trace::Trace() {}

//------------------------------------------------------------------------------

void Trace::VLog(const char *category, const char *fmt, va_list &args) {
  // first prepend the category
  npf_pprintf(&pt_uart_putc, NULL, "[%s] ", category);

  npf_vpprintf(&pt_uart_putc, NULL, fmt, args);
  // end with NL+CR as thats how it previously worked using stdio' printf
  npf_pprintf(&pt_uart_putc, NULL, "\r\n");
}

//------------------------------------------------------------------------------

void Trace::Log(const char *category, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  VLog(category, fmt, args);
  va_end(args);
}

//------------------------------------------------------------------------------

void Trace::Debug(const char *fmt, ...) {
#ifndef NDEBUG
  va_list args;
  va_start(args, fmt);
  VLog("-D-", fmt, args);
  va_end(args);
#endif
}

//------------------------------------------------------------------------------

void Trace::Error(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  VLog("*ERROR*", fmt, args);
  va_end(args);
}

//------------------------------------------------------------------------------
