#include "SamplePool.h"
#include <string.h>
#include <stdlib.h>
#include "System/Console/Trace.h"
#include "Application/Persistency/PersistencyService.h" 
#include "System/io/Status.h"
#include <string>
#include "SoundFontSample.h"
#include "SoundFontPreset.h"
#include "SoundFontManager.h"
#include "Application/Model/Config.h"

#define SAMPLE_LIB "root:samplelib" 

SamplePool::SamplePool() {
	for (int i=0;i<MAX_PIG_SAMPLES;i++) {
		names_[i]=NULL ;
		wav_[i]=NULL ;
	} ;
	count_=0 ;
} ;

SamplePool::~SamplePool() {
	for (int i=0;i<MAX_PIG_SAMPLES;i++) {
		SAFE_DELETE(wav_[i]) ;
		SAFE_FREE(names_[i]) ;
	} ;
} ;

const char *SamplePool::GetSampleLib() {
	Config *config=Config::GetInstance() ;
	const char *lib=config->GetValue("SAMPLELIB") ;
	return lib?lib:SAMPLE_LIB ;
} 

void SamplePool::Reset() {
	count_=0 ;
	for (int i=0;i<MAX_PIG_SAMPLES;i++) {
		SAFE_DELETE(wav_[i]) ;
		SAFE_FREE(names_[i]) ;
	} ;
	SoundFontManager::GetInstance()->Reset() ;
} ;

void SamplePool::Load() {

	Path sampleDir("samples:");

	I_Dir *dir=FileSystem::GetInstance()->Open(sampleDir.GetPath().c_str()) ;
	if (!dir) {
		return ;
	}

	// First, find all wav files

	dir->GetContent("*.wav") ;
	IteratorPtr<Path> it(dir->GetIterator()) ;
	count_=0 ;

	for(it->Begin();!it->IsDone();it->Next()) {
		Path &path=it->CurrentItem() ;
//		Trace::Dump("Got sample name '%s'",name) ;
		loadSample(path.GetPath().c_str()) ;
		if (count_==MAX_PIG_SAMPLES) {
		   Trace::Error("Warning maximum sample count reached") ;
		   break ;
		} ;

	} ;

	// now, let's look at soundfonts

	dir->GetContent("*.sf2") ;
	IteratorPtr<Path> it2(dir->GetIterator()) ;

	for(it2->Begin();!it2->IsDone();it2->Next()) {
		Path &path=it2->CurrentItem() ;
		loadSoundFont(path.GetPath().c_str()) ;
	} ;

	delete dir ;

	// now sort the samples

	int rest=count_ ;
	while(rest>0) {
		int index=0 ;
		for (int i=1;i<rest;i++) {
			if (strcmp(names_[i],names_[index])>0) {
				index=i ;
			};
		} ;
		SoundSource *tWav=wav_[index] ;
		char *tName=names_[index] ;
		wav_[index]=wav_[rest-1] ;
		names_[index]=names_[rest-1] ;
		wav_[rest-1]=tWav;
		names_[rest-1]=tName ;
		rest-- ;
	} ;
} ;

SoundSource *SamplePool::GetSource(int i) {
	return wav_[i] ;
} ;

char **SamplePool::GetNameList() {
	return names_ ;
} ;

int SamplePool::GetNameListSize() {
	return count_ ;
} ;

bool SamplePool::loadSample(const char *path) {

	if (count_==MAX_PIG_SAMPLES) return false ;

	Path sPath(path) ;
    Status::Set("Loading %s",sPath.GetName().c_str()) ;

	Path wavPath(path) ;
	WavFile *wave=WavFile::Open(path) ;
	if (wave) {
		wav_[count_]=wave ;
		const std::string name=wavPath.GetName() ;
		names_[count_]=(char*)SYS_MALLOC(name.length()+1) ;
		strcpy(names_[count_],name.c_str()) ;
		count_++ ;
		wave->GetBuffer(0,wave->GetSize(-1)) ;
		wave->Close() ;
		return true ;
	} else {
		Trace::Error("Failed to load samples %s",wavPath.GetName().c_str()) ;
		return false ;
 	}
}

#define IMPORT_CHUNK_SIZE 1000

int SamplePool::ImportSample(Path &path) {

	if (count_==MAX_PIG_SAMPLES) return -1 ;

	// construct target path

	std::string dpath="samples:" ;
	dpath+=path.GetName() ;
	Path dstPath(dpath.c_str()) ;

    // Opens files

	I_File *fin=FileSystem::GetInstance()->Open(path.GetPath().c_str(),"r") ;
	if (!fin) {
		Trace::Error("Failed to open input file %s",path.GetPath().c_str()) ;
		return -1;
	} ;
	fin->Seek(0,SEEK_END) ;
	long size=fin->Tell() ;
	fin->Seek(0,SEEK_SET) ;

	I_File *fout=FileSystem::GetInstance()->Open(dstPath.GetPath().c_str(),"w") ;
	if (!fout) {
		fin->Close() ;
		delete (fin) ;
		return -1 ;
	} ;

	// copy file to current project

	char buffer[IMPORT_CHUNK_SIZE] ;
	while (size>0) {
		int count=(size>IMPORT_CHUNK_SIZE)?IMPORT_CHUNK_SIZE:size ;
		fin->Read(buffer,1,count) ;
		fout->Write(buffer,1,count) ;
		size-=count ;
	} ;

	fin->Close() ;
	fout->Close() ;
	delete(fin) ;
	delete(fout) ;

	// now load the sample

	bool status=loadSample(dstPath.GetPath().c_str()) ;

	SetChanged() ;
	SamplePoolEvent ev ;
	ev.index_=count_-1 ;
	ev.type_=SPET_INSERT ;
	NotifyObservers(&ev) ;
	return status?(count_-1):-1 ;
};

void SamplePool::PurgeSample(int i) {

	// construct the path of the sample to delete

	std::string wavPath="samples:" ;
	wavPath+=names_[i] ;
	Path path(wavPath.c_str()) ;
	//delete wav
	SAFE_DELETE(wav_[i]) ;
	// delete name entry
	SAFE_DELETE(names_[i]) ;

	// delete file
	FileSystem::GetInstance()->Delete(path.GetPath().c_str()) ;

	// shift all entries from deleted to end
	for (int j=i;j<count_-1;j++) {
		wav_[j]=wav_[j+1] ;
		names_[j]=names_[j+1] ;
	} ;
	// decrease sample count
	count_-- ;
	wav_[count_]=0 ;
	names_[count_]=0 ;

	// now notify observers
	SetChanged() ;
	SamplePoolEvent ev ;
	ev.index_=i ;
	ev.type_=SPET_DELETE ;
	NotifyObservers(&ev) ;
} ;

bool SamplePool::loadSoundFont(const char *path) {

	sfBankID  id=SoundFontManager::GetInstance()->LoadBank(path) ;
	if (id==-1) {
		return false ;
	} 

	// Grab the sample offset

	long offset=sfGetSMPLOffset(id) ;

	// Add all presets of the sf

	WORD presetCount=0 ;
	SFPRESETHDRPTR pHeaders=sfGetPresetHdrs(id,&presetCount); 

	for (int i=0;i<presetCount;i++) {
		if (count_<MAX_PIG_SAMPLES) {
			sfPresetHdr current=pHeaders[i] ;
			wav_[count_]=new SoundFontPreset(id,i) ;
			const char *name=pHeaders[i].achPresetName ;
			names_[count_]=(char*)SYS_MALLOC(strlen(name)+1) ;
			strcpy(names_[count_],name) ;
			count_++ ;
		}
	}
/*
	// Get Sample information

	WORD headerCount=0 ;
	SFSAMPLEHDRPTR  &headers=sfGetSampHdrs(id,&headerCount ); 

	// Loop on every sample, add them

	for (int i=0;i<headerCount;i++) {
		if (count_<MAX_PIG_SAMPLES) {
			sfSampleHdr &current=headers[i] ;
			wav_[count_]=new SoundFontSample(current) ;
			const char *name=headers[i].achSampleName ;
			names_[count_]=(char*)SYS_MALLOC(strlen(name)+1) ;
			strcpy(names_[count_],name) ;
			count_++ ;
		}
	}
*/	return true ;
} ;
