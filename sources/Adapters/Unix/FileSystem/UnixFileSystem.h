
#ifndef _UNIX_FILESYSTEM_H_ 
#define _UNIX_FILESYSTEM_H_ 

#include "System/FileSystem/FileSystem.h"
#include <stdio.h>

class UnixFile: public I_File {
public:
	UnixFile(FILE *) ;
	virtual int Read(void *ptr, int size, int nmemb) ;
	virtual int Write(const void *ptr, int size, int nmemb) ;
	virtual void Printf(const char *format,...);
	virtual void Seek(long offset,int whence) ;
	virtual long Tell() ;
	virtual void Close() ;
private:
	FILE *file_ ;
} ;

class UnixDir: public I_Dir {
public:
    UnixDir(const char *path) ;
	virtual ~UnixDir() {} ;
    virtual void GetContent(char *mask) ;
	virtual void GetProjectContent() ;
} ;

class UnixFileSystem: public FileSystem {
public:
    UnixFileSystem() ;
	virtual I_File *Open(const char *path,char *mode);
	virtual I_Dir *Open(const char *path) ;
	virtual Result MakeDir(const char *path) ;	
	virtual void Delete(const char *path) ;
	virtual FileType GetFileType(const char *path) ;
} ;
#endif
