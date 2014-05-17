#include "Phrase.h"
#include "System/System/System.h"
#include <stdlib.h>
#include <string.h>

Phrase::Phrase() {

	int size=PHRASE_COUNT*16 ; // PHRASE_COUNT phrases of 0x10 steps
	note_=(unsigned char *)SYS_MALLOC(size) ;
	memset(note_,0xFF,size) ;
	instr_=(unsigned char *)SYS_MALLOC(size) ;
	memset(instr_,0xFF,size) ;

	cmd1_=(FourCC *)SYS_MALLOC(size*sizeof(FourCC)) ;
	memset(cmd1_,'-',size*sizeof(FourCC)) ;
	param1_=(unsigned short *)SYS_MALLOC(size*sizeof(short)) ;
	memset(param1_,0x00,size*sizeof(short)) ;

	cmd2_=(FourCC *)SYS_MALLOC(size*sizeof(FourCC)) ;
	memset(cmd2_,'-',size*sizeof(FourCC)) ;
	param2_=(unsigned short *)SYS_MALLOC(size*sizeof(short)) ;
	memset(param2_,0x00,size*sizeof(short)) ;

	for (int i=0;i<PHRASE_COUNT;i++) {
		isUsed_[i]=false ;
	}
} ;

Phrase::~Phrase() {
	if (note_) SYS_FREE(note_) ;
	if (instr_) SYS_FREE(instr_) ;
    /* CMDS_HERE
	if (cmd_) SYS_FREE(cmd_) ;
	if (cmdData_) SYS_FREE(cmdData_) ;
	*/
} ;

unsigned short Phrase::GetNext() {
	for (int i=0;i<PHRASE_COUNT;i++) {
		if (!isUsed_[i]) {
			isUsed_[i]=true ;
			return i ;
		}
	}
	return NO_MORE_PHRASE ;
} ;

void Phrase::SetUsed(unsigned char c) {
	isUsed_[c]=true ;
}

void Phrase::ClearAllocation() {

	for (int i=0;i<PHRASE_COUNT;i++) {
		isUsed_[i]=false ;
	}
} ;
