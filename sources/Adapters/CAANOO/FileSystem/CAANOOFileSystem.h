
#ifndef _CAANOO_FILESYSTEM_H_ 
#define _CAANOO_FILESYSTEM_H_ 

#include "System/FileSystem/FileSystem.h"
#include <stdio.h>

class CAANOOFile: public I_File {
public:
	CAANOOFile(FILE *) ;
	virtual int Read(void *ptr, int size, int nmemb) ;
	virtual int Write(const void *ptr, int size, int nmemb) ;
	virtual void Printf(const char *format,...);
	virtual void Seek(long offset,int whence) ;
	virtual long Tell() ;

	virtual void Close() ;
private:
	FILE *file_ ;
} ;

class CAANOODir: public I_Dir {
public:
    CAANOODir(const char *path) ;
	virtual ~CAANOODir() {} ;
    virtual void GetContent(char *mask) ;
} ;

class CAANOOFileSystem: public FileSystem {
public:
	virtual I_File *Open(const char *path,char *mode);
	virtual I_Dir *Open(const char *path) ;
	virtual FileType GetFileType(const char *path) ;	
	virtual void MakeDir(const char *path) ;	
	virtual void Delete(const char *path) ;	
} ;
#endif
