#ifndef _RTTYPABLEINFO_H_
#define _RTTYPABLEINFO_H_

class RTTypable ;

// This class is defining a framework for handling class information
// At the moment it provides basic type recognition to avoid using dynamic
// cast and run type information as provided by the compiler

class RTTypeInfo {
public:

	// Constructor

	RTTypeInfo() ;

	// Registration flag handling. Used by/for the system

	void SetRegistered() ;
	bool IsRegistered() ;

	// Should return the info type of the subclass (if needed)
	// At the moment, inheritance from multiple RTTypable are not
	// supported

	virtual RTTypeInfo *GetSubTypeInfo()=0 ;

private:
	bool _registered ; // The registration variable
} ;

#endif