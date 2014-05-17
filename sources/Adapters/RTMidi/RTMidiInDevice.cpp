#include "RTMidiInDevice.h"
#include "Services/Midi/MidiMessage.h"
#include "System/Console/n_assert.h"

void mycallback( double deltatime, std::vector< unsigned char > *message, void *userData )
{
	unsigned int nBytes = message->size();
	if (nBytes>3) 
  {
    NAssert("Didn't expect more than 3 bytes, fix code underneath");
    return ;
  }
	RTMidiInDevice *dev=(RTMidiInDevice *)userData ;
  MidiMessage msg ;
  msg.status_ = message->at(0) ;
  msg.data1_  = (nBytes> 1) ? message->at(1) :0 ;
  msg.data2_  = (nBytes> 2) ? message->at(2) :0 ;
  dev->SendDriverMessage(msg) ;
}

RTMidiInDevice::RTMidiInDevice(int index,const char *name):MidiInDevice(name),index_(index) {
};

RTMidiInDevice::~RTMidiInDevice() {
};

bool RTMidiInDevice::initDriver() {

	rtMidiIn_.openPort(index_) ;
	rtMidiIn_.setCallback( &mycallback,this );

	// ignore sysex, timing, or active sensing messages.
	rtMidiIn_.ignoreTypes( true, true, true );

	return true ;
} ;

void RTMidiInDevice::closeDriver() {
	rtMidiIn_.closePort() ;
} ;

bool RTMidiInDevice::startDriver() {
	return true ;
};

void RTMidiInDevice::stopDriver() {
};

void RTMidiInDevice::SendDriverMessage(MidiMessage &message) {
	onDriverMessage(message) ;
} ;
