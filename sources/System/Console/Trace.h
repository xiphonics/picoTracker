/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _TRACE_H_
#define _TRACE_H_

#include "Foundation/T_Singleton.h"

class Trace : public T_Singleton<Trace> {
public:
  //--------------------------------------

  class Logger {
  public:
    virtual ~Logger(){};
    virtual void AddLine(const char *) = 0;
  };

  //--------------------------------------

  static void Debug(const char *fmt, ...);
  static void Log(const char *category, const char *fmt, ...);
  static void Error(const char *fmt, ...);
  static void RegisterEtlErrorHandler();

  static void trace_uart_putc(int c, void *context);

  //--------------------------------------

  Trace();

  void AddLine(const char *line);

  Trace::Logger *SetLogger(Trace::Logger &);

protected:
  static void VLog(const char *category, const char *fmt, va_list &args);

private:
  Trace::Logger *logger_;
};

#endif
