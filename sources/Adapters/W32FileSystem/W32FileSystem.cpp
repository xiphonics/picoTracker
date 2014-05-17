
#include "W32FileSystem.h"
#include "Adapters/WSDLSystem/WSDLSystem.h"
#include "System/Console/Trace.h"
#include <windows.h>
#include <string.h>
#include <stdarg.h>

W32File::W32File(FILE *file) {
	file_=file ;
} ;

W32File::~W32File() {
}

int W32File::Read(void *ptr,int size, int nmemb) {
	return fread(ptr,size,nmemb,file_) ;
} ;

int W32File::Write(const void *ptr,int size, int nmemb) {
	return fwrite(ptr,size,nmemb,file_) ;
} ;

void W32File::Printf(const char *fmt, ...) {
     va_list args;
     va_start(args,fmt);

     vfprintf(file_,fmt,args ); 
     va_end(args);
} ;

void W32File::Seek(long offset,int whence) {
	int sk=fseek(file_,offset,whence) ;
	NAssert(sk==0) ;
} ;

long W32File::Tell() {
	return ftell(file_) ;
} ;
void W32File::Close() {
	fclose(file_) ;
} ;
//

W32Dir::W32Dir(const char *path):I_Dir(path) {
} ;

void W32Dir::GetContent(char *mask) {

	Empty() ;
	WIN32_FIND_DATA findData ;
	char fullPath[MAX_PATH + 1] ;
	sprintf(fullPath,"%s/%s",path_,mask) ;
	HANDLE handle=FindFirstFile(fullPath,&findData) ;
	bool more=(handle!=INVALID_HANDLE_VALUE) ;
	while (more) {
		std::string fullpath=path_ ;
		if (path_[strlen(path_)-1]!='/') {
			fullpath+="/" ;
		}
		fullpath+=findData.cFileName ;
		Path *path=new Path(fullpath.c_str()) ;
		Insert(path) ;
		more=FindNextFile(handle,&findData) ;
	}
	FindClose(handle) ;
};


I_Dir *W32FileSystem::Open(const char *path) {
    return new W32Dir(path) ;
} ;

I_File *W32FileSystem::Open(const char *path,char *mode) {
	char *rmode ;
	switch(*mode) {
        case 'r':
            rmode="rb" ;
            break ;
        case 'w':
            rmode="wb" ;
            break ;
        default:
            Trace::Error("Invalid mode: %s",mode) ;
            return 0 ;
    }

	FILE *file=fopen(path,rmode) ;
	W32File *wFile=0 ;
	if (file) {
		wFile=new W32File(file) ;
	}
	return wFile ;
} ;

FileType W32FileSystem::GetFileType(const char *path) {
	DWORD attr=GetFileAttributes(path) ;
	if (attr==INVALID_FILE_ATTRIBUTES) {
		return FT_UNKNOWN ;
	} ;
	return (attr&FILE_ATTRIBUTE_DIRECTORY)?FT_DIR:FT_FILE ;
};

void W32FileSystem::Delete(const char *path) {
	DeleteFile(path) ;	
}

Result W32FileSystem::MakeDir(const char *path) {
	BOOL success = CreateDirectory(path,NULL) ;
  if (!success)
  {
    std::string error = WSDLSystem::SGetLastErrorString();
    return Result(error);
  }
  return Result::NoError;
}