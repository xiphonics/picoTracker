#ifndef _WAV_FILE_WRITER_H_
#define _WAV_FILE_WRITER_H_

#include "System/FileSystem/FileSystem.h"
#include "Application/Utils/fixed.h"

class WavFileWriter {
public:
	WavFileWriter(const char *path) ;
	~WavFileWriter() ;
	void AddBuffer(fixed *,int size) ; // size in samples
	void Close() ;
private:
	int sampleCount_ ;
	short *buffer_ ;
	int bufferSize_ ;
	I_File *file_ ;
} ;
#endif
