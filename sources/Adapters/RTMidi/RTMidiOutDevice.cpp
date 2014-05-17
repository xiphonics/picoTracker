
#include "RTMidiOutDevice.h"
#include "System/Console/Trace.h"


RTMidiOutDevice::RTMidiOutDevice(RtMidiOut &out,int index,const char *name):
	MidiOutDevice(name),
	rtMidiOut_(out),
	running_(false),
	index_(index)
{
} ;

RTMidiOutDevice::~RTMidiOutDevice() {
} ;

bool RTMidiOutDevice::Init(){
	return true ;
} ;

void RTMidiOutDevice::Close(){
}  ;

bool RTMidiOutDevice::Start(){
	rtMidiOut_.openPort( index_ );
	running_=true ;
	return true ;
}  ;

void RTMidiOutDevice::Stop(){
	running_=false ;
	rtMidiOut_.closePort() ;
} ;

void RTMidiOutDevice::SendMessage(MidiMessage &msg)
{
  if (running_)
  {
    std::vector<unsigned char> message;
    message.push_back(msg.status_) ;

    if (msg.data1_ != MidiMessage::UNUSED_BYTE)
    {
      message.push_back(msg.data1_) ;
    }

    if (msg.data2_ != MidiMessage::UNUSED_BYTE)
    {
      message.push_back(msg.data2_) ;
    }
    rtMidiOut_.sendMessage( &message );
  }
}
