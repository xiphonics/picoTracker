#pragma once

#include "System/Errors/Result.h"
#include "Trace.h"

#include <stdio.h>

class StdOutLogger : public Trace::Logger {
  virtual void AddLine(const char *);
};
