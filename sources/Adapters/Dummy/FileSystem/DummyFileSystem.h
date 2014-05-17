
#ifndef _DUMMY_FILESYSTEM_H_ 
#define _DUMMY_FILESYSTEM_H_ 

#include "System/FileSystem/FileSystem.h"
#include <stdio.h>
#include <string.h>


class DummyFileSystem: public FileSystem {
public:
	virtual I_File *Open(const char *path,char *mode);
	virtual I_Dir *Open(const char *path) ;
	virtual FileType GetFileType(const char *path) ;	
	virtual void MakeDir(const char *path) {} ;
	virtual void Delete(const char *) {} ;

} ;
#endif
