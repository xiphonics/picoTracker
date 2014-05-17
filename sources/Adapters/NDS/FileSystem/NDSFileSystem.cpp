
#include "NDSFileSystem.h"
#include "System/io/Trace.h"
#include "Application/Utils/wildcard.h"
#include <sys/dir.h>
#include <fat.h>
#include <ctype.h>
#include <stdarg.h>

NDSDir::NDSDir(const char *path):I_Dir(path) {
} ;

void NDSDir::GetContent(char *mask) {

	Empty() ;
        
    
    // Read directory content

	char fullpath[81] ;
	strcpy(fullpath,((NDSFileSystem *)NDSFileSystem::GetInstance())->root_) ;
	strcat(fullpath,path_) ;

	Trace::Debug("Reading path content for %s",fullpath) ;
 
    DIR_ITER* directory = diropen (fullpath);

	if (directory == NULL) {
		Trace::Dump("Failed to open %s",fullpath) ;
		return ;
	}

    struct stat st;
    char filename[256];

	while (dirnext(directory, filename, &st) == 0) {
 		char current[128] ;
        strcpy(current,filename) ;
        char *c=current ;
		while(*c) {
            *c=tolower(*c) ;
            c++ ;
        }
        Trace::Dump("testing mask") ;
        
        if (wildcardfit (mask,current)) {
            strcpy(current,filename) ;
            Trace::Dump("Inserting %s/%s",path_,current) ;
			sprintf(fullpath,"%s/%s",path_,current) ;
			Path *path=new Path(fullpath) ;
			Insert(path) ;
		} else {
            Trace::Dump("skipping %s",current) ;
        }
	
    } ;   
	dirclose(directory);
	
};

NDSFile::NDSFile(FILE *file) {
	file_=file ;
} ;

int NDSFile::Read(void *ptr,int size, int nmemb) {
	return fread(ptr,size,nmemb,file_) ;
} ;

int NDSFile::Write(const void *ptr,int size, int nmemb) {
	return fwrite(ptr,size,nmemb,file_) ;
} ;

void NDSFile::Printf(const char *fmt, ...) {
     va_list args;
     va_start(args,fmt);

     vfprintf(file_,fmt,args ); 
     va_end(args);
} ;

void NDSFile::Seek(long offset,int whence) {
	fseek(file_,offset,whence) ;
} ;

long NDSFile::Tell() {
	return ftell(file_) ;
} ;
void NDSFile::Close() {
	fflush(file_) ;
//	fsync(fileno(file_)) ;
	fclose(file_) ;
} ;

NDSFileSystem::NDSFileSystem() {
Trace::Debug("init fat") ;
    fatInitDefault();
	strcpy(root_,"/") ;
Trace::Debug("~init fat") ;
}

I_File *NDSFileSystem::Open(const char *path,char *mode) {
	char buffer[256] ;
	strcpy(buffer,root_) ;
	strcat(buffer,path) ;
	char *rmode ;
	switch(*mode) {
        case 'r':
            rmode="rb" ;
            break ;
        case 'w':
            rmode="wb" ;
            break ;
        default:
            Trace::Dump("Invalid mode: %s",mode) ;
            return 0 ;
    }

	FILE *file=fopen(buffer,rmode) ;
	NDSFile *wFile=0 ;
	if (file) {
		wFile=new NDSFile(file) ;
	}
	return wFile ;
} ;

I_Dir *NDSFileSystem::Open(const char *path) {
    return new NDSDir(path) ;
} ;
