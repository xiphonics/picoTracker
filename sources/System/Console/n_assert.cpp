
#include "System/Console/Trace.h"
#include <string.h>
#include <assert.h>

void __NAssert(const char *exp, const char *file, unsigned line) {
	const char *filestr=file ;
	if (strlen(file)>20) {
		filestr=file+strlen(file)-20 ;
	}
	Trace::Error("Assertion failed: %s",exp) ;
	Trace::Error("  >> file [%s]",filestr) ;
	Trace::Error("  >> line %d",line) ;
  assert(0);
} ;
