#include "SoundFontPreset.h"
#include "Externals/Soundfont/ENAB.H"
#include "System/Console/Trace.h"

SoundFontPreset::SoundFontPreset(int sfID,int presetID):
	sfID_(sfID),
	presetID_(presetID),
	vect_(0),
	lastNote_(-1)
{
	navigator_.SetHydraFont(GetHydraPtr(sfID));   // would be bank select
} ;

SoundFontPreset::~SoundFontPreset() {
}

int SoundFontPreset::GetChannelCount(int note) {
	checkNote(note) ;
	// Until I merge the L & R samples
	return 1 ;
} ;

void *SoundFontPreset::GetSampleBuffer(int note){
	checkNote(note) ;
	if (vect_) {
		return (void *)vect_->dwStart ;
	} ;
	return 0 ;
} ;

int SoundFontPreset::GetSampleRate(int note) {
	checkNote(note) ;
	if (vect_) {
		return vect_->dwSampleRate ;
	} ;
	return 44100 ;
} ;

int SoundFontPreset::GetSize(int note) {
	checkNote(note) ;
	if (vect_) {
		return vect_->dwEnd ;
	} ;
	return 0 ;
} ;

int SoundFontPreset::GetRootNote(int note) {
	checkNote(note) ;
	if (vect_) {
		twoByteUnion tbu ;
		tbu.wVal=vect_->shOrigKeyAndCorr ;
		return tbu.byVals.by1;
	} ;
	return 60 ;
} ;

bool SoundFontPreset::IsMulti() {
	return true ;
} ;

bool SoundFontPreset::IsLooped(int note) {
	checkNote(note) ;
	return ((vect_->shSampleModes&0x1)!=0) ;
} ;

int SoundFontPreset::GetLoopStart(int note) {
	checkNote(note) ;
	if (vect_) {
		return (IsLooped(note))?vect_->dwStartloop:-1 ;
	}
	return -1 ;
} ;

int SoundFontPreset::GetLoopEnd(int note) {
	checkNote(note) ;
	if (vect_) {
		return vect_->dwEndloop ;
	}
	return -1 ;
} ;

void SoundFontPreset::checkNote(int note) {
	if (note!=lastNote_) {
		navigator_.Navigate( presetID_, note, 127 ); 
		int oscCount=navigator_.GetNOsc() ;
		if (oscCount!=0) {
			vect_= navigator_.GetSFPtr();		
		}
		lastNote_=note ;
	}
} ;

