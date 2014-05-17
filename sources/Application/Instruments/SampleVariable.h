
#ifndef _SAMPLE_VARIABLE_H_
#define _SAMPLE_VARIABLE_H_

#include "Foundation/Variables/WatchedVariable.h"
#include "Foundation/Observable.h"

class SampleVariable:public WatchedVariable,public I_Observer {
public:
	SampleVariable(const char *name,FourCC id) ;
	~SampleVariable() ;
protected:
    virtual void Update(Observable &o,I_ObservableData *d) ;

} ;
#endif