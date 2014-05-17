#include "System/System/System.h"
#include "Trace.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


Trace::Trace() 
:logger_(0)
{
}


//------------------------------------------------------------------------------

void Trace::AddLine(const char* line)
{
  if (logger_)
  {
    logger_->AddLine(line);    
  } 
  else
  {
	printf("Trying to log %s before logger is installed",line);
  }
}


//------------------------------------------------------------------------------

Trace::Logger *Trace::SetLogger(Trace::Logger& logger)
{
  Trace::Logger *tmp = logger_;
  logger_ = &logger;
  return tmp;
}


//------------------------------------------------------------------------------

void Trace::VLog(const char* category,  const char *fmt, const va_list& args) 
{
  char buffer[4096] ;
  sprintf(buffer, "[%s] ",category);
  
  char *ptr = buffer+strlen(buffer);
  
  vsprintf(ptr,fmt,args ); 
  GetInstance()->AddLine(buffer) ;
  
}

//------------------------------------------------------------------------------

void Trace::Log(const char* category, const char *fmt, ...) 
{
  va_list args;
  va_start(args,fmt);
  VLog(category, fmt, args);
  va_end(args);
}


//------------------------------------------------------------------------------

void Trace::Debug(const char *fmt, ...) 
{
#ifndef NDEBUG
  va_list args;
  va_start(args,fmt);
  VLog("-D-",fmt, args);
  va_end(args);
#endif  
}


//------------------------------------------------------------------------------

void Trace::Error(const char *fmt, ...) 
{
  va_list args;
  va_start(args,fmt);
  VLog("*ERROR*",fmt, args);
  va_end(args);
}


//------------------------------------------------------------------------------
