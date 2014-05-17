
#ifndef _W32_FILESYSTEM_H_ 
#define _W32_FILESYSTEM_H_ 

#include "System/FileSystem/FileSystem.h"
#include <stdio.h>

class W32File: public I_File {
public:
	W32File(FILE *) ;
	virtual ~W32File() ;
	virtual int Read(void *ptr, int size, int nmemb) ;
	virtual int Write(const void *ptr, int size, int nmemb) ;
	virtual void Printf(const char *format,...);
	virtual void Seek(long offset,int whence) ;
	virtual long Tell() ;

	virtual void Close() ;
private:
	FILE *file_ ;
} ;

class W32Dir: public I_Dir {
public:
    W32Dir(const char *path) ;
	virtual ~W32Dir() {} ;
    virtual void GetContent(char *mask) ;
} ;

class W32FileSystem: public FileSystem {
public:
	virtual I_File *Open(const char *path,char *mode);
	virtual I_Dir *Open(const char *path) ;
	virtual FileType GetFileType(const char *path) ;
	virtual Result MakeDir(const char *path) ;
	virtual void Delete(const char *path) ;
} ;
#endif
