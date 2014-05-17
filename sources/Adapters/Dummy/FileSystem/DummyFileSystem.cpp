
#include "DummyFileSystem.h"
#include "system/io/Trace.h"
#include "Application/Utils/wildcard.h"	
#include <ctype.h>



I_File *DummyFileSystem::Open(const char *path,char *mode) {
	return 0 ;
} ;


I_Dir *DummyFileSystem::Open(const char *path) {
    return 0 ;
} ;

FileType DummyFileSystem::GetFileType(const char *path) {
    return FT_UNKNOWN ;
} ;

