#ifndef _CHAIN_H_
#define _CHAIN_H_

#ifndef PICOBUILD
#define CHAIN_COUNT 0xFF
#define NO_MORE_CHAIN 0x100
#else
#define CHAIN_COUNT 0x7F
#define NO_MORE_CHAIN 0x80
#endif

class Chain {
public:
	Chain() ;
	~Chain() ;
	unsigned short GetNext() ;
	bool IsUsed(unsigned char i) { return isUsed_[i] ; } ;
	void SetUsed(unsigned char c) ;
	void ClearAllocation() ;

	unsigned char *data_ ;
	unsigned char *transpose_ ;

	
private:
	bool isUsed_[CHAIN_COUNT] ;

} ;

#endif
