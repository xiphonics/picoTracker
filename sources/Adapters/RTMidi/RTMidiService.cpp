
#include "RTMidiService.h"
#include "RTMidiOutDevice.h"
#include "RTMidiInDevice.h"
#include "System/Console/Trace.h"
#include "Application/Model/Config.h"

//static void CALLBACK MidiFlushProc(UINT uiID, UINT uiMsg, DWORD
//                                  dwUser, DWORD dw1, DWORD dw2) {
//
//	W32MidiService *msvc=(W32MidiService *)dwUser ;
//	msvc->Flush() ;
//} ;


RTMidiService::RTMidiService() {
	const char *delay=Config::GetInstance()->GetValue("MIDIDELAY") ;
	if (delay) {
		midiDelay_=atoi(delay) ;
	} else {
		midiDelay_=0;
	}

	// RtMidiIn constructor
	try {
		rtMidiIn_ = new RtMidiIn();
	}
	catch ( RtError &error ) {
		Trace::Error("Couldn't get RtMidiIn object") ;
		rtMidiIn_=0 ;
		Trace::Error(error.getMessageString());
	}

	// RtMidiOut constructor
	try {
		rtMidiOut_ = new RtMidiOut();
	}
	catch ( RtError &error ) {
		Trace::Error("Couldn't get RtMidiOut object") ;
		rtMidiOut_=0 ;
		Trace::Error(error.getMessageString());
	}

} ;

RTMidiService::~RTMidiService() {
} ;

void RTMidiService::buildDriverList() {// Here we just loop over existing Midi out and create a midi device for each of them.

	// Let's enumerate midi ins


	// Check inputs.

  unsigned int nPorts = (rtMidiIn_)?rtMidiIn_->getPortCount():0;
	Trace::Log("MIDI","%d input port(s)",nPorts) ;

	for (uint i=0; i<nPorts; i++ ) {
		try {
			std::string portName = rtMidiIn_->getPortName(i);
			RTMidiInDevice *in=new RTMidiInDevice(i,portName.c_str()) ;
			Trace::Log("MIDI"," %s",portName.c_str()) ;
			inList_.Insert(in) ;
		} catch (RtError &error) {
      Trace::Error(error.getMessageString());
		}
	}

	// Now check outputs.

	nPorts = (rtMidiOut_)?rtMidiOut_->getPortCount():0;
	Trace::Log("MIDI","%d output port(s)",nPorts) ;
	for (uint i=0; i<nPorts; i++ )
  {
		try 
    {
			std::string portName = rtMidiOut_->getPortName(i);
			RTMidiOutDevice *out=new RTMidiOutDevice(*rtMidiOut_,i,portName.c_str()) ;
			Trace::Log("MIDI"," %s",portName.c_str()) ;
			Insert(out) ;
		} 
    catch (RtError &error) 
    {
			Trace::Error(error.getMessageString());
		}
	}
}

