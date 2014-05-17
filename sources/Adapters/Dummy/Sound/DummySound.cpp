
#include "DummySound.h"

bool DummySound::Init() {

   return true ;
} ; 


void DummySound::Close() {

} ;

bool DummySound::Start() {
	return 1 ;
} ; 

void DummySound::Stop() {
} ;


 // length here is in bytes
 
void DummySound::QueueBuffer(char *buffer,int len) {

}


int DummySound::GetPlayedBufferPercentage() {
	return 0 ;
} ;

int DummySound::GetSampleRate(){
	return 44100 ;
} ;

