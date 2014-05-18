
#include "PSPFileSystem.h"
#include "System/Console/Trace.h"
#include <string.h>
#include <stdarg.h>
#include <string>
#include <Application/utils/wildcard.h>

#include <sys/dir.h>
#include <sys/stat.h>

PSPFile::PSPFile(SceUID file) {
	file_=file ;
	writeBufferPos_=0 ;
}

PSPFile::~PSPFile() {
}

int PSPFile::Read(void *ptr,int size, int nmemb) {
	return sceIoRead(file_,ptr,size*nmemb);
}

void PSPFile::flush() {
	if (writeBufferPos_>0) {
		sceIoWrite(file_,writeBuffer_,writeBufferPos_);
	}
	writeBufferPos_=0 ;
}

int PSPFile::Write(const void *ptr,int size, int nmemb) {
	int len=size*nmemb ;
	if (writeBufferPos_+len>WRITE_BUFFER_SIZE) {
		flush() ;
	}
	if (len>WRITE_BUFFER_SIZE) {
		sceIoWrite(file_,ptr,len);
	} else {
		memcpy(writeBuffer_+writeBufferPos_,ptr,len) ;
		writeBufferPos_+=len ;
	}
	return  len ;
}

void PSPFile::Printf(const char *fmt, ...) {
	char buffer[1024] ;
    va_list args;
    va_start(args,fmt);

    vsprintf(buffer,fmt,args );
	Write(buffer,strlen(buffer),1) ;	
    va_end(args);
}

void PSPFile::Seek(long offset,int whence) {
	sceIoLseek(file_,offset,whence);
}

long PSPFile::Tell() {
	return 	sceIoLseek(file_,0,SEEK_CUR);
}

void PSPFile::Close() {
	flush() ;
	sceIoClose(file_) ;
}
//

PSPDir::PSPDir(const char *path):I_Dir(path) {
}

void PSPDir::GetContent(char *mask) {

	Empty() ;

  SceIoDirent de;
  memset(&de,0,sizeof(SceIoDirent));

  SceUID fd=sceIoDopen(path_);
  if(fd<0) {
    Trace::Error("Failed to open %s",path_);
		return;
	}
	
    SceUID v=sceIoDread(fd,&de);
	char nameBuffer[256] ;
	
    while(v!=0) {
	
		// See if matches current mask
		int len=strlen(de.d_name) ;
		for (int i=0;i<len;i++) {
			nameBuffer[i]=tolower(de.d_name[i]) ;
		}
		nameBuffer[len]=0 ;
		
		if (wildcardfit(mask,nameBuffer)) {

			std::string fullpath=path_ ;
			if (path_[strlen(path_)-1]!='/') {
				fullpath+="/" ;
			}
			fullpath+=de.d_name ;
		
			Path *path=new Path(fullpath.c_str()) ;
			Insert(path) ;

		}
//		sceIoClose(v) ;
        v=sceIoDread(fd,&de);
    }
	
	sceIoDclose(fd) ;

};


I_Dir *PSPFileSystem::Open(const char *path) {
    return new PSPDir(path) ;
}

I_File *PSPFileSystem::Open(const char *path,char *mode) {
	
	int flags=0 ;
	
	switch(*mode) {
        case 'r':
            flags=PSP_O_RDONLY ;
            break ;
        case 'w':
            flags=PSP_O_WRONLY|PSP_O_CREAT ;
            break ;
        default:
            return 0 ;
    }

	SceUID file=sceIoOpen(path,flags, 0777) ;

	PSPFile *pspFile=0 ;
	if (file>0) {
		pspFile=new PSPFile(file) ;
	}
	return pspFile ;
}

FileType PSPFileSystem::GetFileType(const char *path) {

	struct stat attributes ;
	if (stat(path,&attributes)==0)
	{
		if (attributes.st_mode&S_IFDIR) return FT_DIR ;
		if (attributes.st_mode&S_IFREG) return FT_FILE ;
	}
	else
	{
		if (!strcmp("ms0:", path))
		{
			return FT_DIR;
		}

	}

	return FT_UNKNOWN ;

}

void PSPFileSystem::Delete(const char *path) {
	sceIoRemove(path);
}

Result PSPFileSystem::MakeDir(const char *path) {
	int retval = sceIoMkdir(path,0777);
  if (retval != 0)
  {
    std::ostringstream oss ;
    oss << "MakeDir failed with code " << retval;
    return Result(oss.str());
  }
  return Result::NoError;
}