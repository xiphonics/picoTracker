
#include "UnixFileSystem.h"
#include "System/Console/Trace.h"
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

#include "Application/Utils/wildcard.h"	

UnixDir::UnixDir(const char *path):I_Dir(path) {
} ;

void UnixDir::GetContent(char *mask) {

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
  //      Trace::Dump("testing mask") ;
        
        if (wildcardfit (mask,current)) {
            strcpy(current,entry->d_name) ;
//            Trace::Dump("Inserting %s/%s",path_,current) ;
			std::string fullpath=path_ ;
			if (path_[strlen(path_)-1]!='/') {
				fullpath+="/" ;
			}
			fullpath+=current ;
			Path *path=new Path(fullpath.c_str()) ;
			Insert(path) ;
		} else {
  //          Trace::Dump("skipping %s",current) ;
        }
	
    } ;   
	closedir (directory);
	
};

void UnixDir::GetProjectContent() {
	
	Empty() ;
	const char* mask = (const char *) "*";
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
		//      Trace::Dump("testing mask") ;
        
        if (wildcardfit (mask,current)) {
            strcpy(current,entry->d_name) ;
			//            Trace::Dump("Inserting %s/%s",path_,current) ;
			
			
			//we dont want any dot files but we do want .. 
			if(current[0] == '.' && current[1] != '.'){
				//de nada
			} else {
				std::string fullpath=path_ ;
				if (path_[strlen(path_)-1]!='/') {
					fullpath+="/" ;
				}
				fullpath+=current ;
				Path *path=new Path(fullpath.c_str()) ;
				Insert(path) ;
			}
		} else {
			//          Trace::Dump("skipping %s",current) ;
        }
		
    } ;   
	closedir (directory);
} ;

UnixFile::UnixFile(FILE *file) {
	file_=file ;
} ;

int UnixFile::Read(void *ptr,int size, int nmemb) {
	return fread(ptr,size,nmemb,file_) ;
} ;

int UnixFile::Write(const void *ptr,int size, int nmemb) {
	return fwrite(ptr,size,nmemb,file_) ;
} ;

void UnixFile::Printf(const char *fmt, ...) {
     va_list args;
     va_start(args,fmt);

     vfprintf(file_,fmt,args ); 
     va_end(args);
} ;

void UnixFile::Seek(long offset,int whence) {
	fseek(file_,offset,whence) ;
} ;

long UnixFile::Tell() {
	return ftell(file_) ;
} ;
void UnixFile::Close() {
	fflush(file_) ;
	fsync(fileno(file_)) ;
	fclose(file_) ;
} ;

UnixFileSystem::UnixFileSystem() {
}

I_File *UnixFileSystem::Open(const char *path,char *mode) {
	char *rmode ;
	switch(*mode) {
        case 'r':
            rmode=(char *)"rb" ;
            break ;
        case 'w':
            rmode=(char *)"wb" ;
            break ;
        default:
            Trace::Error("Invalid mode: %s",mode) ;
            return 0 ;
    }

	FILE *file=fopen(path,rmode) ;
	UnixFile *wFile=0 ;
	if (file) {
		wFile=new UnixFile(file) ;
	}
	return wFile ;
} ;

I_Dir *UnixFileSystem::Open(const char *path) {
    return new UnixDir(path) ;
} ;

FileType UnixFileSystem::GetFileType(const char* path) {

	struct stat attributes ;
	if (stat(path,&attributes)==0) {
		if (attributes.st_mode&S_IFDIR) return FT_DIR ;
		if (attributes.st_mode&S_IFREG) return FT_FILE ;
	}
	return FT_UNKNOWN ;

} ;

void UnixFileSystem::Delete(const char *path) {
	remove(path) ;
} ;

Result UnixFileSystem::MakeDir(const char *path) {
  
	int success = mkdir(path,S_IRWXU) ;
  if (success <0)
  {
    std::ostringstream oss;
    oss << "Could not create path " << path << " (errno:" << errno << ")";
    return Result(oss.str());
  }
  return Result::NoError;	
} ;	
