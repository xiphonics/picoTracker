
#ifndef _DINGOO_FILESYSTEM_H_ 
#define _DINGOO_FILESYSTEM_H_ 

#include "System/FileSystem/FileSystem.h"
#include <stdio.h>

class DINGOOFile: public I_File {
public:
	DINGOOFile(FILE *) ;
	virtual int Read(void *ptr, int size, int nmemb) ;
	virtual int Write(const void *ptr, int size, int nmemb) ;
	virtual void Printf(const char *format,...);
	virtual void Seek(long offset,int whence) ;
	virtual long Tell() ;

	virtual void Close() ;
private:
	FILE *file_ ;
} ;

class DINGOODir: public I_Dir {
public:
    DINGOODir(const char *path) ;
	virtual ~DINGOODir() {} ;
    virtual void GetContent(char *mask) ;
} ;

class DINGOOFileSystem: public FileSystem {
public:
	virtual I_File *Open(const char *path,char *mode);
	virtual I_Dir *Open(const char *path) ;
	virtual FileType GetFileType(const char *path) ;	
	virtual void MakeDir(const char *path) ;	
	virtual void Delete(const char *path) ;	
} ;
#endif
