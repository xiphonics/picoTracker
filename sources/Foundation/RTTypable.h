
#ifndef _RTTYPABLE_H_
#define _RTTYPABLE_H_


#include "Foundation/RTTypeInfo.h"
#include <stdio.h>

// This class is an abstract class providing a framework for limited
// run time class checking. Allow to provide dynamic cast like functionalities
// without relying on the compiler's one

class RTTypable {

	// We protect the constructor since this class is abstract

protected:
	
	// Constructor

	RTTypable() ;

	// Destructor

	virtual ~RTTypable() ;

	// The typable class should implement this function and
	// return the associated type info

	virtual RTTypeInfo &GetInfo()=0 ;

public:

	// This function returns wheter the typable class belongs to the
	// specified typable class info. Note that at the moment the test
	// is based on pointer comparison on the RTTypeInfo associated to
	// the class so each RTTypeInfo should be singletons.

	inline bool IsInstanceOf(RTTypeInfo &i) {
		bool isOne=false ;
		RTTypeInfo *info=&this->GetInfo() ;
		while (info!=NULL) {
			if (info==&i) {
				return true ;
			}
			info=info->GetSubTypeInfo() ;
		}
		return false ;	
	};

	// This function is used to register type infos

	static void Register(RTTypeInfo &) ;
} ;

#endif