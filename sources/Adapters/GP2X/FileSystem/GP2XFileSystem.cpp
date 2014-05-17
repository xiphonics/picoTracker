
#include "GP2XFileSystem.h"
#include "System/Console/Trace.h"
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
//#include <sys/dirent.h>
#include "Application/Utils/wildcard.h"	

GP2XDir::GP2XDir(const char *path):I_Dir(path) {
} ;

void GP2XDir::GetContent(char *mask) {

	Empty() ;

 
	DIR* directory; 
	struct dirent* entry; 

 
	directory = opendir (path_); 
	if (directory == NULL) {
		Trace::Error("Failed to open %s",path_) ;
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

GP2XFile::GP2XFile(FILE *file) {
	file_=file ;
} ;

int GP2XFile::Read(void *ptr,int size, int nmemb) {
	return fread(ptr,size,nmemb,file_) ;
} ;

int GP2XFile::Write(const void *ptr,int size, int nmemb) {
	return fwrite(ptr,size,nmemb,file_) ;
} ;

void GP2XFile::Printf(const char *fmt, ...) {
     va_list args;
     va_start(args,fmt);

     vfprintf(file_,fmt,args ); 
     va_end(args);
} ;

void GP2XFile::Seek(long offset,int whence) {
	fseek(file_,offset,whence) ;
} ;

long GP2XFile::Tell() {
	return ftell(file_) ;
} ;
void GP2XFile::Close() {
	fflush(file_) ;
	fsync(fileno(file_)) ;
	fclose(file_) ;
} ;

I_File *GP2XFileSystem::Open(const char *path,char *mode) {
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
	GP2XFile *wFile=0 ;
	if (file) {
		wFile=new GP2XFile(file) ;
	}
	return wFile ;
} ;

I_Dir *GP2XFileSystem::Open(const char *path) {
    return new GP2XDir(path) ;
} ;

FileType GP2XFileSystem::GetFileType(const char *path) {

    struct stat attributes ;
    if (stat(path,&attributes)==0) {
        if (attributes.st_mode&S_IFDIR) return FT_DIR ;
        if (attributes.st_mode&S_IFREG) return FT_FILE ;
    }        
    return FT_UNKNOWN ;
} ;

void GP2XFileSystem::Delete(const char *path) {
    remove(path) ;
} ;

Result GP2XFileSystem::MakeDir(const char *path) {
  
	int success = mkdir(path,S_IRWXU) ;
  if (success <0)
  {
    std::ostringstream oss;
    oss << "Could not create path " << path << " (errno:" << errno << ")";
    return Result(oss.str());
  }
  return Result::NoError;	
} 