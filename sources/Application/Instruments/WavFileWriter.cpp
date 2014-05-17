#include "WavFileWriter.h"
#include "System/Console/Trace.h"

WavFileWriter::WavFileWriter(const char *path):
	file_(0),
	buffer_(0),
	bufferSize_(0),
	sampleCount_(0)
{
	Path filePath(path) ;
	file_=FileSystem::GetInstance()->Open(filePath.GetPath().c_str(),"wb") ;
	if (file_) {

		// RIFF chunk

		unsigned int chunk ;
		chunk=Swap32(0x46464952) ;
		file_->Write(&chunk,1,4);
		unsigned int size ;
		size=0 ; // to be filled later
		file_->Write(&size,1,4);

		// WAVE chunk

		chunk=Swap32(0x45564157) ;
		file_->Write(&chunk,1,4);
		chunk=Swap32(0x20746D66) ;
		file_->Write(&chunk,1,4);
		size=Swap32(16) ;
		file_->Write(&size,1,4);

		unsigned short ushort ;
		ushort=Swap16(1) ; // compression
		file_->Write(&ushort,1,2);
		ushort=Swap16(2) ; // nChannels
		file_->Write(&ushort,1,2);
		unsigned int sampleRate=Swap32(44100) ;
		file_->Write(&sampleRate,1,4);

		unsigned int byteRate=Swap32(4*44100) ;
		file_->Write(&byteRate,1,4);

		ushort=Swap16(4) ; //  blockalign
		file_->Write(&ushort,1,2);

		ushort=Swap16(16) ; // bitPerSample
		file_->Write(&ushort,1,2);

		// data subchunk

		chunk = Swap32(0x61746164);
		file_->Write(&chunk,1,4);

		size=0 ;  // to be updated later
		file_->Write(&chunk,1,4);
	} ;
} ;

WavFileWriter::~WavFileWriter() {
	Close();
}

void WavFileWriter::AddBuffer(fixed *bufferIn,int size) {

	if (!file_) return ;

	// allocate a short buffer for transfer

	if (size>bufferSize_) {
		SAFE_FREE(buffer_) ;
		buffer_=(short *)malloc(size*2*sizeof(short)) ;
		bufferSize_=size ;
	} ;

	if (!buffer_) return ;

	short *s=buffer_ ;
	fixed *p=bufferIn ;

	fixed v;
	fixed f_32767=i2fp(32767) ;
	fixed f_m32768=i2fp(-32768) ;

	for (int i=0;i<size*2;i++) {
        // Left
		v=*p++  ;
		if (v>f_32767) {
			v=f_32767 ;
		} else if (v<f_m32768) {
			v=f_m32768 ;
		}
		*s++=short(fp2i(v)) ;
	} ;
	file_->Write(buffer_,2,size*2) ;
	sampleCount_+=size ;
} ;

void WavFileWriter::Close() {

	if (!file_) return ;

	size_t len=file_->Tell() ;
	len=Swap32(len-8) ;
	file_->Seek(4,SEEK_SET) ;
	file_->Write(&len,4,1) ;

	file_->Seek(40,SEEK_SET) ;
	sampleCount_=Swap32(sampleCount_*4) ;
	file_->Write(&sampleCount_,4,1) ;

	file_->Seek(0,SEEK_END) ;

	file_->Close() ;
	SAFE_DELETE(file_) ;
	SAFE_FREE(buffer_) ;

} ;