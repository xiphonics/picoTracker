#include "PersistencyService.h"
#include "Persistent.h"
#include "Externals/Compression/lz.h"
#include "System/Console/Trace.h"
#include "Foundation/Types/Types.h"

PersistencyService::PersistencyService():Service(MAKE_FOURCC('S','V','P','S')) {
} ;

void PersistencyService::Save() {

	Path filename("project:lgptsav.dat") ;

	TiXmlDocument doc(filename.GetPath()) ;
	TiXmlElement first("LITTLEGPTRACKER") ;
	TiXmlNode *node=doc.InsertEndChild(first) ;

	// Loop on all registered service
	// accumulating XML flow
	
	IteratorPtr<SubService> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Persistent *currentItem=(Persistent *)&it->CurrentItem() ;
		currentItem->Save(node) ;
	} ;

	doc.SaveFile() ;
} ;

bool PersistencyService::Load() {

	Path filename("project:lgptsav.dat") ;
	PersistencyDocument doc( filename.GetPath() );

	// Try opening the file
	
	FileSystem *fs=FileSystem::GetInstance() ;
	I_File *file=fs->Open(filename.GetPath().c_str(),"r") ;
	if (!file) return false ;
	
	// get file size and read all buffer
	
	file->Seek(0,SEEK_END) ;
	int length=file->Tell() ;

	unsigned char *compBuffer=(unsigned char *)SYS_MALLOC(length) ;

  file->Seek(0,SEEK_SET) ;
	file->Read(compBuffer,1,length) ;
	file->Close();
	delete file ;
	
	if (!doc.Parse((char *)compBuffer)) {
        
		// Get uncompressed buffer size from first byte
		
		int offset=sizeof(int) ;
		int fullLength ;
		memcpy(&fullLength,compBuffer,offset) ;
		
		// Allocate a buffer to decompress data
		
		unsigned char *xmlSource=(unsigned char *)SYS_MALLOC(fullLength) ;
		if (!xmlSource) {
			Trace::Error("could not allocate space for %d bytes") ;
			return false ;
		}

    LZ_Uncompress(compBuffer+offset,xmlSource,length-offset);

		// Initialize XML document on decompressed buffer
		doc.Parse((char *)xmlSource) ;

		SYS_FREE(xmlSource) ;

		
	} ; 
	SYS_FREE(compBuffer) ;

	TiXmlNode* node = 0;
	node = doc.FirstChild( "LITTLEGPTRACKER" );
	if (!node) {
		Trace::Error("could not find master node") ;
		return false ;
	};

	TiXmlElement* element =node->ToElement();
	node = element->FirstChildElement() ;
	if (node) {
		element = node->ToElement();
		while (element) {
			IteratorPtr<SubService> it(GetIterator()) ;
			for (it->Begin();!it->IsDone();it->Next()) {
				Persistent *currentItem=(Persistent *)&it->CurrentItem() ;
				if (currentItem->Restore(element)) {
					break ;
				} ;
			}
			element = element->NextSiblingElement();
		} ;
	}
	return true ;
} ;
