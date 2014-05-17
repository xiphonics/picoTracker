#include "SoundFontSample.h"

SoundFontSample::SoundFontSample(sfSampleHdr &header):sfHeader_(header) {
}

SoundFontSample::~SoundFontSample() {
}

void *SoundFontSample::GetSampleBuffer(int note) {
	return (void *)sfHeader_.dwStart ;
}

int SoundFontSample::GetChannelCount(int note) {
	return 1;
} ;

int SoundFontSample::GetSampleRate(int note) {
	return sfHeader_.dwSampleRate ;
} ;

int SoundFontSample::GetSize(int note){
	return sfHeader_.dwEnd ;
} ;

int SoundFontSample::GetRootNote(int note) {
	return sfHeader_.byOriginalKey ;
} ;

int SoundFontSample::GetLoopStart(int note) {
	return sfHeader_.dwStartloop ;
} ;

int SoundFontSample::GetLoopEnd(int note) {
	return sfHeader_.dwEndloop ;
} ;