#include "Status.h"
#include <stdio.h>
#include <stdarg.h>
//#include <windows.h>

void Status::Set(char *fmt, ...) {

	Status *status=Status::GetInstance() ;
	if (!status) return ;

     char buffer[4096] ;
     va_list args;
     va_start(args,fmt);

     vsprintf(buffer,fmt,args ); 
     status->Print(buffer) ; 

     va_end(args);

}
