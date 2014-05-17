
#include "DINGOOFileSystem.h"
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <ctype.h>
//#include <sys/dirent.h>
#include "Application/Utils/wildcard.h"	

DINGOODir::DINGOODir(const char *path):I_Dir(path) {
} ;

void DINGOODir::GetContent(char *mask) {

	Empty() ;

 
	DIR* directory; 
	struct dirent* entry; 

 
	directory = opendir (path_); 
	if (directory == NULL) {
		return ;
	}

	while ((entry = readdir (directory)) != NULL)
  {
 		char current[128] ;
    strcpy(current,entry->d_name) ;
    char *c=current ;
		while(*c) 
    {
      *c=tolower(*c) ;
      c++ ;
    }
        
    if (wildcardfit (mask,current)) 
    {
      strcpy(current,entry->d_name) ;
			std::string fullpath=path_ ;
			if (path_[strlen(path_)-1]!='/') 
      {
				fullpath+="/" ;
			}
			fullpath+=current ;
			Path *path=new Path(fullpath.c_str()) ;
			Insert(path) ;
		}
	
  } ;   
	closedir (directory);
};

DINGOOFile::DINGOOFile(FILE *file) {
	file_=file ;
} ;

int DINGOOFile::Read(void *ptr,int size, int nmemb) {
	return fread(ptr,size,nmemb,file_) ;
} ;

int DINGOOFile::Write(const void *ptr,int size, int nmemb) {
	return fwrite(ptr,size,nmemb,file_) ;
} ;

void DINGOOFile::Printf(const char *fmt, ...) {
     va_list args;
     va_start(args,fmt);

     vfprintf(file_,fmt,args ); 
     va_end(args);
} ;

void DINGOOFile::Seek(long offset,int whence) {
	fseek(file_,offset,whence) ;
} ;

long DINGOOFile::Tell() {
	return ftell(file_) ;
} ;
void DINGOOFile::Close() {
	fflush(file_) ;
	fsync(fileno(file_)) ;
	fclose(file_) ;
} ;

I_File *DINGOOFileSystem::Open(const char *path,char *mode) {
	char *rmode ;
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
	DINGOOFile *wFile=0 ;
	if (file) {
		wFile=new DINGOOFile(file) ;
	}
	return wFile ;
} ;

I_Dir *DINGOOFileSystem::Open(const char *path) {
    return new DINGOODir(path) ;
} ;

FileType DINGOOFileSystem::GetFileType(const char *path) {

    struct stat attributes ;
    if (stat(path,&attributes)==0) {
        if (attributes.st_mode&S_IFDIR) return FT_DIR ;
        if (attributes.st_mode&S_IFREG) return FT_FILE ;
    }        
    return FT_UNKNOWN ;
} ;

void DINGOOFileSystem::Delete(const char *path) {
    remove(path) ;
} ;

void DINGOOFileSystem::MakeDir(const char *path) {
	mkdir(path,S_IRWXU) ;
} ;	
