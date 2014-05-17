#ifndef _T_STACK_H_
#define _T_STACK_H_

#include "T_SimpleList.h"

template <class Item>
class T_Stack: protected T_SimpleList<Item> {
public:
	T_Stack(bool isOwner=false) ;
	virtual ~T_Stack() {} ;
	void Push(Item &) ;
	Item *Pop(bool lifo=false) ;
	virtual void Insert(Item&) ;
	virtual void Insert(Item*) ;
	void Empty() ;
	int Size() { return _size ; } ;
private:
	int _size ;
	bool _lifo ;
} ;

#include "T_Stack.cpp"     // Include the implementation file.

#endif
