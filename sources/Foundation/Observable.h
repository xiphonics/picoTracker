
#pragma once

#include <vector>

// Data to be passed from the observable to the observer

class I_ObservableData {
} ;

// The observer: Simply allows to be notified with data

class Observable ;

class I_Observer {
public:
	virtual ~I_Observer() {} ;
    virtual void Update(Observable &o,I_ObservableData *d)=0 ;
} ;

// The observable

class Observable {
public:
	Observable() ;
	virtual ~Observable() ;
	void AddObserver(I_Observer &o) ;
	void RemoveObserver(I_Observer &o) ;
	void RemoveAllObservers() ;
	int  CountObservers() ;

	inline void NotifyObservers() { NotifyObservers(0) ; } ;

	void NotifyObservers(I_ObservableData *d) ;

	void SetChanged() ;
	inline void ClearChanged() { _hasChanged=false ; } ;
	bool HasChanged() ;
private:
	std::vector<I_Observer *> _list ;
	bool _hasChanged ;
};


