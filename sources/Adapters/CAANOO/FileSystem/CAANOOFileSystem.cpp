
#include "CAANOOFileSystem.h"
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <ctype.h>
//#include <sys/dirent.h>
#include "Application/Utils/wildcard.h"	

CAANOODir::CAANOODir(const char *path):I_Dir(path) {
} ;

void CAANOODir::GetContent(char *mask) {

	Empty() ;

 
	DIR* directory; 
	struct dirent* entry; 

 
	directory = opendir (path_); 
	if (directory == NULL) {
		return ;
	}

	while ((entry = readdir (directory)) != NULL) {
 		char current[128] ;
        strcpy(current,entry->d_name) ;
        char *c=current ;
		while(*c) {
            *c=tolower(*c) ;
            c++ ;
        }
        
        if (wildcardfit (mask,current)) {
            strcpy(current,entry->d_name) ;
			std::string fullpath=path_ ;
			if (path_[strlen(path_)-1]!='/') {
				fullpath+="/" ;
			}
			fullpath+=current ;
			Path *path=new Path(fullpath.c_str()) ;
			Insert(path) ;
		} else {
        }
	
    } ;   
	closedir (directory);
	
};

CAANOOFile::CAANOOFile(FILE *file) {
	file_=file ;
} ;

int CAANOOFile::Read(void *ptr,int size, int nmemb) {
	return fread(ptr,size,nmemb,file_) ;
} ;

int CAANOOFile::Write(const void *ptr,int size, int nmemb) {
	return fwrite(ptr,size,nmemb,file_) ;
} ;

void CAANOOFile::Printf(const char *fmt, ...) {
     va_list args;
     va_start(args,fmt);

     vfprintf(file_,fmt,args ); 
     va_end(args);
} ;

void CAANOOFile::Seek(long offset,int whence) {
	fseek(file_,offset,whence) ;
} ;

long CAANOOFile::Tell() {
	return ftell(file_) ;
} ;
void CAANOOFile::Close() {
	fflush(file_) ;
	fsync(fileno(file_)) ;
	fclose(file_) ;
} ;

I_File *CAANOOFileSystem::Open(const char *path,char *mode) {
	const char *rmode ;
	switch(*mode) {
        case 'r':
            rmode="rb" ;
            break ;
        case 'w':
            rmode="wb" ;
            break ;
        default:
            return 0 ;
    }

	FILE *file=fopen(path,rmode) ;
	CAANOOFile *wFile=0 ;
	if (file) {
		wFile=new CAANOOFile(file) ;
	}
	return wFile ;
} ;

I_Dir *CAANOOFileSystem::Open(const char *path) {
    return new CAANOODir(path) ;
} ;

FileType CAANOOFileSystem::GetFileType(const char *path) {

    struct stat attributes ;
    if (stat(path,&attributes)==0) {
        if (attributes.st_mode&S_IFDIR) return FT_DIR ;
        if (attributes.st_mode&S_IFREG) return FT_FILE ;
    }        
    return FT_UNKNOWN ;
} ;

void CAANOOFileSystem::Delete(const char *path) {
    remove(path) ;
} ;

void CAANOOFileSystem::MakeDir(const char *path) {
	mkdir(path,S_IRWXU) ;
} ;	
