
#ifndef _GP2X_FILESYSTEM_H_ 
#define _GP2X_FILESYSTEM_H_ 

#include "System/FileSystem/FileSystem.h"
#include <stdio.h>

class GP2XFile: public I_File {
public:
	GP2XFile(FILE *) ;
	virtual int Read(void *ptr, int size, int nmemb) ;
	virtual int Write(const void *ptr, int size, int nmemb) ;
	virtual void Printf(const char *format,...);
	virtual void Seek(long offset,int whence) ;
	virtual long Tell() ;

	virtual void Close() ;
private:
	FILE *file_ ;
} ;

class GP2XDir: public I_Dir {
public:
    GP2XDir(const char *path) ;
	virtual ~GP2XDir() {} ;
    virtual void GetContent(char *mask) ;
} ;

class GP2XFileSystem: public FileSystem {
public:
	virtual I_File *Open(const char *path,char *mode);
	virtual I_Dir *Open(const char *path) ;
	virtual FileType GetFileType(const char *path) ;	
	virtual Result MakeDir(const char *path) ;	
	virtual void Delete(const char *path) ;	
} ;
#endif
