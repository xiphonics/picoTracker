#ifndef _T_FACTORY_H_
#define _T_FACTORY_H_

//
// Encapsulate a Factory pattern allowing the installation
// of different factories matching one given interface with
// only one available at the time
//

template <class Item>
class T_Factory {
protected:
	virtual ~T_Factory<Item>() {} ;
public :

    // Install the factory to use

	static void Install(Item *) ;

	// Get the currently installed factory

	static Item *GetInstance() ;

protected:

	// The static instance of the singleton

	static Item * instance_ ;
} ;

#include "T_Factory.cpp"

#endif
