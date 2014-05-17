#ifndef _POINTERHASHKEY_H_
#define _POINTERHASHKEY_H_

#include "I_HashKey.h" 

//
// PointerHashKey: An implementation of a Hashtable key that uses a pointer
// as unique key. See I_Hashkey

class PointerHashKey: public I_Hashkey {
public:

	// Constructor: takes a pointer as key

	PointerHashKey(void *) ;

	// Returns the hashcode of the key

	virtual unsigned long HashCode() ;

	// Returns whether the two keys are equals

	virtual bool Equals(const I_Hashkey &) ;

	// Returns a deep copy of the key

	virtual I_Hashkey& Clone() ;

private:
	unsigned long _key ;
} ;

#endif