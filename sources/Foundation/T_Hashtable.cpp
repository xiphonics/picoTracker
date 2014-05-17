#include "T_HashtableIterator.h"

//
// Implementation of the Hashtable. Influenced by code found on the WWW
//  see http://jmvidal.ece.sc.edu/352/PS10/HashTable.cpp.html

// Constructor

template <class Item>
T_Hashtable<Item>::T_Hashtable() {
	_content=new T_Vector<HashEntry>(nextPrime(101)) ; // Initializes the hashtable to roughly 100
	Empty() ;
}

// Destructor

template <class Item>
T_Hashtable<Item>::~T_Hashtable() {
	delete _content ;
}

// Cleans the content of the hashtable

template <class Item>
void T_Hashtable<Item>::Empty() {
	for (int i=0;i<_content->Size();i++) {
		_content->Get(i).SetInfo(EMPTY) ;
	}
	_size=0 ;
}

// Puts an item associtated to a key

template <class Item>
void T_Hashtable<Item>::Put(I_Hashkey &key,Item &item) {
	int currentPos=findPos(key) ;
	if (isActive(currentPos)) { // if the cell exists and is active, we keep out
		return ;
	}

	HashEntry he (&key,&item,ACTIVE) ;
	_content->Put(currentPos, he) ;
	if (++_size > _content->Size()/2) {
		rehash() ;
	}
}

// Removes the item associated to the specified key

template <class Item>
void T_Hashtable<Item>::Remove(I_Hashkey &key) {
	int currentPos=findPos(key) ;
	if (isActive(currentPos)) {
		_content->Get(currentPos).SetInfo(DELETED) ;
	}
}

// Returns the element associated to the specified key or NULL if it is not existing

template <class Item>
Item* T_Hashtable<Item>::Get(I_Hashkey &key) {
	int currentPos=findPos(key) ;
	if (isActive(currentPos)) {
		HashEntry &e=_content->Get(currentPos) ;
		return e.GetItem() ;
	}
	return NULL ;
}

// Returns true if the specified cell is active

template <class Item>
bool T_Hashtable<Item>::isActive(int index) {
	return _content->Get(index).GetInfo()==ACTIVE ;
}

// Returns the index of the item location according to its key

template <class Item>
int T_Hashtable<Item>::findPos(I_Hashkey &key) {

    int collisionNum = 0;
    int currentPos = key.HashCode() % _content->Size( );
    while( _content->Get(currentPos).GetInfo() != EMPTY &&
           (! _content->Get(currentPos).GetKey()->Equals(key)) )
    {
        currentPos += 2 * ++collisionNum - 1;  // Compute ith probe
        if( currentPos >= _content->Size( ) )
            currentPos -= _content->Size( );
    }
    return currentPos;

}

// Resize the internal storage to be the next prime number after twice the 
// current internal storage size

template <class Item>
void T_Hashtable<Item>::rehash() {

    T_Vector<HashEntry> *oldContent=_content ;
	_content=new T_Vector<HashEntry>(nextPrime(2*_content->Size())) ;
	Empty() ;
	for (int i=0;i<oldContent->Size();i++) {
		HashEntry &ae=oldContent->Get(i) ;
		if (ae.GetInfo()==ACTIVE) {
			Put(*ae.GetKey(),*ae.GetItem()) ;
		}
	}
}

// Returns true if the specified number is prime... not efficient

template <class Item>
bool T_Hashtable<Item>::isPrime(int n) {

    if( n == 2 || n == 3 )
        return true;

    if( n == 1 || n % 2 == 0 )
        return false;

    for( int i = 3; i * i <= n; i += 2 )
        if( n % i == 0 )
            return false;

    return true;
}

// Returns the next prime number after the specified one... not efficient

template <class Item>
int T_Hashtable<Item>::nextPrime(int n) {

    if( n % 2 == 0 )
        n++;

    for( ; !isPrime( n ); n += 2 )
        ;

    return n;
}

// Returns an iterator on the hashtable content

template <class Item>
T_HashtableIterator<Item> *T_Hashtable<Item>::GetIterator() {
	return new T_HashtableIterator<Item>(*this) ;
}