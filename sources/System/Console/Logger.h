#pragma once

#include "Trace.h"
#include "System/Errors/Result.h"
#include "System/FileSystem/FileSystem.h"

#include <stdio.h>

class StdOutLogger: public Trace::Logger
{
  virtual void AddLine(const char*);
};

class FileLogger: public Trace::Logger
{
public:
  FileLogger(const Path& path);
  ~FileLogger();

  Result Init();

private:
  virtual void AddLine(const char*);
  Path path_;
  FILE *file_;
};