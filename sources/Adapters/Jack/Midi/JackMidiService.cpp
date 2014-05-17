
#include "JackMidiService.h"
#include "JackMidiDevice.h"
#include "System/io/Trace.h"
#include "Adapters/Jack/Client/JackClient.h"
#include <jack/jack.h>

JackMidiService::JackMidiService() {
} ;

JackMidiService::~JackMidiService() {
} ;

void JackMidiService::buildDriverList() {// Here we just loop over existing Midi out and create a midi device for each of them.

	JackClient *jc=JackClient::GetInstance() ;
	jack_client_t* client=jc->GetClient() ;
	JackMidiDevice *device=new JackMidiDevice(client,"midiout") ;
	Insert(device) ;
//	jc->Insert(device) ;
} ;

void JackMidiService::SetFrameOffset(int offset) {
     IteratorPtr<MidiDevice>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         JackMidiDevice &current=(JackMidiDevice &)it->CurrentItem() ;
         current.SetFrameOffset(offset) ;
     }
} ;

void JackMidiService::OnNewFrame() {
     IteratorPtr<MidiDevice>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         JackMidiDevice &current=(JackMidiDevice &)it->CurrentItem() ;
         current.OnNewFrame() ;
     }} ;