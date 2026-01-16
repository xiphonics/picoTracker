/*
 * Test stub for host-only builds.
 */
#ifndef _TRACE_H_
#define _TRACE_H_

class Trace {
public:
  static void Debug(const char *fmt, ...) { (void)fmt; }
  static void Log(const char *category, const char *fmt, ...) {
    (void)category;
    (void)fmt;
  }
  static void Error(const char *fmt, ...) { (void)fmt; }
};

#endif
