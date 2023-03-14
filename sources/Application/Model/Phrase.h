#ifndef _PHRASE_H_
#define _PHRASE_H_

#include "Foundation/Types/Types.h"
#ifndef PICOBUILD
#define PHRASE_COUNT 0xFF
#define NO_MORE_PHRASE 0x100
#else
#define PHRASE_COUNT 0x80
#define NO_MORE_PHRASE 0x81
#endif

class Phrase {
public:
	Phrase() ;
	~Phrase() ;
	unsigned short GetNext() ;
	bool IsUsed(uchar i) { return isUsed_[i] ; } ;
	void SetUsed(uchar c) ;
	void ClearAllocation() ;

	uchar *note_ ;
	uchar *instr_ ;
	FourCC *cmd1_ ;
	ushort *param1_ ;
	FourCC *cmd2_ ;
	ushort *param2_ ;
	
private:
	bool isUsed_[PHRASE_COUNT] ;

} ;

#endif
