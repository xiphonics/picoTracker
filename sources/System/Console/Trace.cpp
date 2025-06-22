/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Trace.h"
#include "Externals/etl/include/etl/error_handler.h"
#include "platform.h"
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

// Never inline, so you can breakpoint on this function to get backtraces
__attribute__((noinline)) void EtlError(const etl::exception &e) {
  Trace::Error("ETL: %s:%d: %s", e.file_name(), e.line_number(), e.what());
}

void Trace::RegisterEtlErrorHandler() {
#ifdef ETL_LOG_ERRORS
  etl::error_handler::set_callback<EtlError>();
#endif
}

//------------------------------------------------------------------------------
