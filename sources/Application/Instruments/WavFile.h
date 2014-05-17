
#ifndef _WAV_FILE_H_
#define _WAV_FILE_H_

#include "System/FileSystem/FileSystem.h"
#include "SoundSource.h"

class WavFile:public SoundSource {

protected: // Factory - see Load method
	WavFile(I_File *file) ;
public:
	virtual ~WavFile() ;
	static WavFile *Open(const char *) ;
	virtual void *GetSampleBuffer(int note) ;
	virtual int GetSize(int note) ;
	virtual int GetSampleRate(int note) ;
	virtual int GetChannelCount(int note) ;
	virtual int GetRootNote(int note) ;
	bool GetBuffer(long start,long sampleCount) ; // values in smples
	void Close() ;
	virtual bool IsMulti() {return false ; } ;

protected:
	long readBlock(long position,long count) ;
private:
	I_File *file_ ;  // File
	void *readBuffer_ ; // Temp read buffer
	int readBufferSize_; // Read buffer size
	short *samples_ ; // sample buffer size (16 bits)
	int sampleBufferSize_ ;
	int size_ ; // number of samples
	int sampleRate_ ; // sample rate
	int channelCount_ ; // mono / stereo
	int bytePerSample_ ; // original file is in 8/16bit
	int dataPosition_ ; // offset in file to get to data

	static int bufferChunkSize_ ;
	static bool initChunkSize_ ;
} ;
#endif
