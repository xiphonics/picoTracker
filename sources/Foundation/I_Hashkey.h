
#ifndef _I_HASHKEY_H_
#define _I_HASHKEY_H_

//
// I_Hashkey: Interface that defines the operation to be
// supported by a class used as key to reference object
// in a T_Hashtable
//

class I_Hashkey {
public:

	// Returns a unique code for the key

	virtual unsigned long HashCode()=0 ;

	// Defines an comparison mechanism we can
	// overload with virtual function. Pay attention
	// that you can recieve all kinds of key types, be
	// sure to test the key is of the type you are expecting

	virtual bool Equals(const I_Hashkey &)=0 ;

	// Defines a duplication mechanism we can
	// overload with virtual function. Implementation
	// should return a copy of the current object

	virtual I_Hashkey &Clone()=0 ;

	// Just to make sure these are not used

private:
	bool operator==(const I_Hashkey &) ;
	bool operator!=(const I_Hashkey &) ;
} ;

#endif