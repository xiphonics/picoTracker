
#include "PointerHashKey.h"
#include <stdio.h>

//
// Implementation file for PointerHashKey: a Hashtable key that uses a pointer
// as identifier.

PointerHashKey::PointerHashKey(void *ptr) {
	_key=(long)ptr ; // we convert the pointer into an unsigned long
}

unsigned long PointerHashKey::HashCode() {
	return _key ;
}

bool PointerHashKey::Equals(const I_Hashkey &other) {

	// When implementing Equals, we have first to make sure the key
	// is a PointerHashKey as it might be of any other type

    const I_Hashkey *optr=&other ;
	const PointerHashKey *o=dynamic_cast<const PointerHashKey *>(optr) ;
	if (o) {
		return o->_key==_key ;
	}
	return false ;
}

I_Hashkey& PointerHashKey::Clone() {
	PointerHashKey* newKey=new PointerHashKey(NULL) ;
	newKey->_key=_key ;
	return *newKey ;
}