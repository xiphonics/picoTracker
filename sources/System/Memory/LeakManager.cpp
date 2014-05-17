
#include "LeakManager.h"
#include "Sequencer/Model/IO/Trace.h"

LeakManager LeakManager::_instance ;

LeakManager::LeakManager():_list(true) {
	
}

LeakManager::~LeakManager() {
	IteratorPtr<LeakElement> it(_list.GetIterator()) ;
	int count=0 ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Trace::Dump("Unreleased pointer: %4.4d - 0x%x\n",count++,it->CurrentItem().GetElement()) ;
	}
}

LeakManager &LeakManager::GetInstance() {
	return _instance ;
}

#define TRAP 0x3a1cec8

void LeakManager::AddElement(void *e) {
	if (e==(void*)TRAP) {
		Trace::Dump("Got it !\n") ;
	}
	_list.Insert(new LeakElement(e)) ;
}

void LeakManager::RemoveElement(void *e) {
	IteratorPtr<LeakElement> it(_list.GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		if (it->CurrentItem().GetElement()==e) {
			_list.Remove(it->CurrentItem()) ;
			return ;
		}
	}
}