
#ifndef _NDS_FILESYSTEM_H_ 
#define _NDS_FILESYSTEM_H_ 

#include "System/FileSystem/FileSystem.h"
#include <stdio.h>

class NDSFile: public I_File {
public:
	NDSFile(FILE *) ;
	virtual int Read(void *ptr, int size, int nmemb) ;
	virtual int Write(const void *ptr, int size, int nmemb) ;
	virtual void Printf(const char *format,...);
	virtual void Seek(long offset,int whence) ;
	virtual long Tell() ;

	virtual void Close() ;
private:
	FILE *file_ ;
} ;

class NDSDir: public I_Dir {
public:
    NDSDir(const char *path) ;
	virtual ~NDSDir() {} ;
    virtual void GetContent(char *mask) ;
} ;

class NDSFileSystem: public FileSystem {
public:
    NDSFileSystem() ;
	virtual I_File *Open(const char *path,char *mode);
	virtual I_Dir *Open(const char *path) ;
    char root_[1024] ;
} ;
#endif
