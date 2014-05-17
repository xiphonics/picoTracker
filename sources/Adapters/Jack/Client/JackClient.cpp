#ifndef _NO_JACK_
#include "JackClient.h"
#include "System/System/System.h"
#include "System/Console/Trace.h"

static void jack_error (const char *desc) {
	Trace::Debug("JACK error: %s\n", desc);
}

int jack_callback(jack_nframes_t nframes, void *arg) {

	JackClient *client=(JackClient *)arg ;
	client->ProcessCallback(nframes) ;
	return 0 ;
}
	
static char clientName[64] ;

JackClient::JackClient() {
} ;

JackClient::~JackClient() {
	Trace::Debug("[J]: close driver") ;
	jack_client_close (client_);
} ;

bool JackClient::Init() {

	long time=System::GetInstance()->GetClock() ;
	sprintf(clientName,"littlegptracker-%ld",time) ;
	
// Initialise callbacks

	jack_set_error_function (jack_error);

	client_ = jack_client_new(clientName) ;
	if (!client_) {
		return false ;
	}
	jack_set_process_callback (client_, jack_callback, this);
	return true ;
} ;

bool JackClient::Available() {
	return client_!=0 ;
} ;


jack_client_t *JackClient::GetClient() {
	return client_ ;
} ;

void JackClient::ProcessCallback(jack_nframes_t nframes) {
     IteratorPtr<JackProcessor>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         JackProcessor &current=it->CurrentItem() ;
         current.ProcessCallback(nframes) ;
     }
} ;
#endif
