
#ifndef _LEAK_MANAGER_H_
#define _LEAK_MANAGER_H_

#include "Sequencer/Model/Foundation/T_SimpleList.h"

class LeakElement {
public:
	LeakElement(void *e) { _e=e ; } ;
	void *GetElement() { return _e ; } ;
private:
	void *_e ;
} ;

class LeakManager {
public:
	LeakManager() ;
	~LeakManager() ;
public:
	static LeakManager &GetInstance() ;
	void AddElement(void *e) ;
	void RemoveElement(void *e) ;
private:
	T_SimpleList<LeakElement> _list ;
	static LeakManager _instance ;
} ;
#endif
