
#include "WavFile.h"
#include "System/Console/Trace.h"
#include "Foundation/Types/Types.h"
#include "Services/Time/TimeService.h"
#include "Application/Model/Config.h"
#include <stdlib.h>

int WavFile::bufferChunkSize_=-1 ;
bool WavFile::initChunkSize_=true ;

short Swap16 (short from)
{
#ifdef __ppc__
	short result;
	((char*)&result)[0] = ((char*)&from)[1];
	((char*)&result)[1] = ((char*)&from)[0];
	return  result;
#else
	return from;
#endif	
}

int Swap32 (int from)
{
#ifdef __ppc__
	int result;
	((char*)&result)[0] = ((char*)&from)[3];
	((char*)&result)[1] = ((char*)&from)[2];
	((char*)&result)[2] = ((char*)&from)[1];
	((char*)&result)[3] = ((char*)&from)[0];			 
	return  result;
#else
	return from;
#endif 	
}


WavFile::WavFile(I_File *file) {
	if (initChunkSize_) {
		const char *size=Config::GetInstance()->GetValue("SAMPLELOADCHUNKSIZE") ;
		if (size) {
			bufferChunkSize_=atoi(size) ;
		}
		initChunkSize_=false;
	}
	samples_=0 ;
	size_=0 ;
	readBuffer_=0 ;
	readBufferSize_=0 ;
	sampleBufferSize_=0 ;
	file_=file ;
} ;

WavFile::~WavFile() {
	if (file_) {
		file_->Close() ;
		delete file_ ;
	}
	SAFE_FREE(samples_) ;
	SAFE_FREE(readBuffer_) ;
} ;

WavFile *WavFile::Open(const char *path) {

    // open file

	FileSystem *fs=FileSystem::GetInstance() ;
	I_File *file=fs->Open(path,"r") ;
	
	if (!file) return 0 ;

	WavFile *wav=new WavFile(file) ;

        
        // Get data
        
/*        file->Seek(0,SEEK_SET) ;
        file->Read(fileBuffer,filesize,1) ;
        uchar *ptr=fileBuffer ;*/
        
//Trace::Dump("Loading sample from %s",path) ;

	long position=0 ;

	// Read 'RIFF'

	unsigned int chunk ;

	position+=wav->readBlock(position,4) ;
	memcpy(&chunk,wav->readBuffer_,4) ;
	chunk = Swap32(chunk);
		
	if (chunk!=0x46464952) {
		Trace::Error("Bad RIFF format %x",chunk) ;
		delete(wav) ;
		return 0 ;
	}


	// Read size

	unsigned int size ;
	position+=wav->readBlock(position,4) ;
	memcpy(&size,wav->readBuffer_,4) ;
	size = Swap32(size);

	// Read WAVE

	position+=wav->readBlock(position,4) ;
	memcpy(&chunk,wav->readBuffer_,4) ;
	chunk = Swap32(chunk);

	if (chunk!=0x45564157) {
		Trace::Error("Bad WAV format") ;
		delete wav ;
		return 0 ;
	}

		// Read fmt

	position+=wav->readBlock(position,4) ;
	memcpy(&chunk,wav->readBuffer_,4) ;
	chunk = Swap32(chunk);
		
	if (chunk!=0x20746D66) {
		Trace::Error("Bad WAV/fmt format") ;
		delete wav ;
		return 0 ;
	}

	// Read subchunk size

	position+=wav->readBlock(position,4) ;
	memcpy(&size,wav->readBuffer_,4) ;
	size = Swap32(size);

	if (size<16) {
		Trace::Error("Bad fmt size format") ;
		delete wav ;
		return 0 ;
	}
	int offset=size-16 ;

	// Read compression

	unsigned short comp ;
	position+=wav->readBlock(position,2) ;
	memcpy(&comp,wav->readBuffer_,2) ;
	comp = Swap16(comp);

	if (comp!=1) {
		Trace::Error("Unsupported compression") ;
		delete wav ;
		return 0 ;
	}

	// Read NumChannels (mono/Stereo)

	unsigned short nChannels ;
	position+=wav->readBlock(position,2) ;
	memcpy(&nChannels,wav->readBuffer_,2) ;
	nChannels = Swap16(nChannels);

	// Read Sample rate 

	unsigned int sampleRate ;

	position+=wav->readBlock(position,4) ;
	memcpy(&sampleRate,wav->readBuffer_,4) ;
	sampleRate = Swap32(sampleRate);

	// Skip byteRate & blockalign

	position+=6 ;

	short bitPerSample ;
	position+=wav->readBlock(position,2) ;
	memcpy(&bitPerSample,wav->readBuffer_,2) ;
	bitPerSample = Swap16(bitPerSample);
		
	if ((bitPerSample!=16)&&(bitPerSample!=8)) {
		Trace::Error("Only 8/16 bit supported") ;
		delete wav ;
		return 0 ;
	} ;
	bitPerSample/=8 ;
	wav->bytePerSample_=bitPerSample ;

	// some bad files have bigger chunks

	if (offset) {
		position+=offset ;
	}

	// read data subchunk header
	//Trace::Dump("data subch") ;

	position+=wav->readBlock(position,4) ;
	memcpy(&chunk,wav->readBuffer_,4) ;
	chunk = Swap32(chunk);
	

	while (chunk!=0x61746164) {
		position+=wav->readBlock(position,4) ;
		memcpy(&size,wav->readBuffer_,4) ;
		size = Swap32(size);

		position+=size ;
		position+=wav->readBlock(position,4) ;
		memcpy(&chunk,wav->readBuffer_,4) ;
		chunk = Swap32(chunk);
	}

        wav->sampleRate_=sampleRate ;
       	wav->channelCount_=nChannels ;

	// Read data size in byte

	position+=wav->readBlock(position,4) ;
	memcpy(&size,wav->readBuffer_,4) ;
	size = Swap32(size);

	wav->size_=size/nChannels/bitPerSample ; // Size in samples (stereo/16bits)

	wav->dataPosition_=position ;

	return wav ;
} ; 

void *WavFile::GetSampleBuffer(int note) {
	return samples_ ;
} ;

int WavFile::GetSize(int note) {
	return size_ ;
} ;

int WavFile::GetChannelCount(int note) {
    return channelCount_ ;
} ;

int WavFile::GetSampleRate(int note) {
    return sampleRate_ ;
} ;

long WavFile::readBlock(long start,long size) {
	if (size>readBufferSize_) {
		SAFE_FREE(readBuffer_) ;
		readBuffer_=SYS_MALLOC(size) ;
		readBufferSize_=size ;
	}
  if (!readBuffer_)
  {
    Trace::Error("Failed to allocate read buffer of size %d",size);
  } 
  else 
  {
  	file_->Seek(start,SEEK_SET) ;
    file_->Read(readBuffer_,size,1) ;
  }
	return size ;
} ;


bool WavFile::GetBuffer(long start,long size) {

	// compute the sample buffer size we need,
	// allocate if needed

	int sampleBufferSize=2*channelCount_*size ;
	if (sampleBufferSize>sampleBufferSize_) {
		SAFE_FREE(samples_) ;
		samples_=(short *)SYS_MALLOC(sampleBufferSize) ;
		sampleBufferSize_=sampleBufferSize ;
	}

  if (!samples_)
  {
    Trace::Error("Failed to allocate %d samples",sampleBufferSize);
  }

	// compute the file buffer size we need to read

	int bufferSize=size*channelCount_*bytePerSample_ ;
	int bufferStart=dataPosition_+start*channelCount_*bytePerSample_ ;

	// Read the buffer but in small chunk to let the system breathe
	// if the files are big

	int count=bufferSize ;
	int offset=0 ;
	char *ptr=(char *)samples_ ;
	int readSize =
   (bufferChunkSize_>0) 
   ? bufferChunkSize_
   : count>4096?4096:count;

	while (count>0) {
		readSize=(count>readSize)?readSize:count ;
		readBlock(bufferStart,readSize) ;
		memcpy(ptr+offset,readBuffer_,readSize) ;
		bufferStart+=readSize ;
		count-=readSize ;
		offset+=readSize ;
		if (bufferChunkSize_>0) TimeService::GetInstance()->Sleep(1) ;
	}


        // expand 8 bit data if needed

	unsigned char *src=(unsigned char *)samples_ ;
	short *dst=samples_ ;
	for (int i=size-1;i>=0;i--) {
		if (bytePerSample_==1) {
			dst[i]=(src[i]-128)*256 ;
		} else {
			*dst=Swap16(*dst) ;
			dst++ ;
			if (channelCount_>1) {
				*dst=Swap16(*dst) ;
				dst++ ;
			}
		}
	} 
	return true ;
} ;

void WavFile::Close() {
	file_->Close() ;
	SAFE_DELETE(file_) ;
	SAFE_FREE(readBuffer_) ;
	readBufferSize_=0 ;
} ;

int WavFile::GetRootNote(int note) {
	return 60 ;
} 
