#include "SoundFontManager.h"
#include "System/System/System.h"
#include "System/FileSystem/FileSystem.h"

SoundFontManager::SoundFontManager() {
} ;

SoundFontManager::~SoundFontManager() {
} ;

void SoundFontManager::Reset() {
	std::vector<void *>::iterator it=sampleData_.begin() ;
	while (it!=sampleData_.end()) {
		SAFE_FREE(*it) ;
		it=sampleData_.erase(it) ;
	} ;
} ;

sfBankID SoundFontManager::LoadBank(const char *path) {

	sfBankID id=sfReadSFBFile((char *)path) ; 
	if (id==-1) {
		return -1 ;
	} 
	// open the file

	I_File *fin=FileSystem::GetInstance()->Open(path,"r") ;
	if (!fin) {
		return false;
	}

	// Grab the sample offset

	long offset=sfGetSMPLOffset(id) ;

	// Grab the sample headerzz

	WORD headerCount=0 ;
	SFSAMPLEHDRPTR  &headers=sfGetSampHdrs(id,&headerCount ); 

	// Loop on every sample, load them and adapt the pointers

	for (int i=0;i<headerCount;i++) {

		sfSampleHdr &current=headers[i] ;

		long from=current.dwStart*2+offset ;
		long to=current.dwEnd*2+offset ;
		
		int byteSize=to-from ;

		void *buffer=malloc(byteSize) ;

		if (buffer) {
			fin->Seek(from,SEEK_SET) ;
			fin->Read(buffer,byteSize,1) ;
		}

		// now adapt the headers so the start is the memory point
		// and all others are sample offset

		current.dwEnd=(current.dwEnd-current.dwStart) ;
		current.dwStartloop=(current.dwStartloop-current.dwStart) ;
		current.dwEndloop=(current.dwEndloop-current.dwStart) ;
		current.dwStart=(DWORD)buffer ;

		sampleData_.push_back(buffer) ;
	}
	fin->Close() ;
	SAFE_DELETE(fin) ;

	return id ;
} ;