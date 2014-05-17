
#include "WatchedVariable.h"

bool WatchedVariable::enabled_=true ;

WatchedVariable::WatchedVariable(const char *name,FourCC id,int value)
                :Variable(name,id,value) {
		updating_=false ;
} ;

WatchedVariable::WatchedVariable(const char *name,FourCC id,bool value)
                :Variable(name,id,value) {
		updating_=false ;
} ;

WatchedVariable::WatchedVariable(const char *name,FourCC id,char **list,int size,int index)
	            :Variable(name,id,list,size,index) {
		updating_=false ;
} ;


void WatchedVariable::onChange() {
	if (!updating_&&enabled_) {
		updating_=true ;
		SetChanged() ;
		NotifyObservers() ;
		updating_=false ;
	}
} ;

void WatchedVariable::Enable() {
	enabled_=true ;
}

void WatchedVariable::Disable() {
	enabled_=false ;
}
