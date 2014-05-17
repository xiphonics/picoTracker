
#ifndef _TRACE_H_
#define _TRACE_H_

#include "Foundation/T_Singleton.h"
#include <stdarg.h>

class Trace: public T_Singleton<Trace>  {
public:

  //--------------------------------------

  class Logger
  {
  public:
    virtual ~Logger() {};
  	virtual void AddLine(const char*) = 0;
  };

  //--------------------------------------

	static void Debug(const char *fmt, ...);
	static void Log(const char* category, const char *fmt, ...);
	static void Error(const char *fmt, ...);

  //--------------------------------------

  Trace();

  void AddLine(const char* line);

  Trace::Logger *SetLogger(Trace::Logger&);

protected:
  static void VLog(const char* category, const char *fmt, const va_list& args); 
private:
  Trace::Logger* logger_;
} ;

#endif
