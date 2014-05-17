
#ifndef _JACK_CLIENT_H_
#define _JACK_CLIENT_H_

#include "jack/jack.h"
#include "Foundation/T_Singleton.h"
#include "Foundation/T_SimpleList.h"

class JackProcessor {
public:
	virtual ~JackProcessor() {} ;
	virtual void ProcessCallback(jack_nframes_t nframes)=0 ; 
} ;

class JackClient: public T_Singleton<JackClient>,public JackProcessor,public T_SimpleList<JackProcessor> {
public:
	JackClient()  ;
	virtual ~JackClient() ;
	bool Init() ;
	bool Available() ;
	jack_client_t* GetClient() ;
	virtual void ProcessCallback(jack_nframes_t nframes) ; 
private:
	jack_client_t* client_ ;
} ;

#endif
