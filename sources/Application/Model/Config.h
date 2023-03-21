#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/Variables/VariableContainer.h"
#include "System/Console/Trace.h"

class Config: public T_Singleton<Config>,public VariableContainer {
public:
	Config() ;
	~Config() ;
	const char *GetValue(const char *key) ;
	void ProcessArguments(int argc,char **argv) ;
} ;

#endif
