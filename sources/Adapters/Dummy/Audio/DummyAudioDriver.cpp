
#include "DummyAudioDriver.h"

bool DummyAudioDriver::InitDriver() {

   return true ;
} ; 


void DummyAudioDriver::CloseDriver() {

} ;

bool DummyAudioDriver::StartDriver() {
	return 1 ;
} ; 

void DummyAudioDriver::StopDriver() {
} ;


int DummyAudioDriver::GetPlayedBufferPercentage() {
	return 0 ;
} ;

int DummyAudioDriver::GetSampleRate(){
	return 44100 ;
} ;

