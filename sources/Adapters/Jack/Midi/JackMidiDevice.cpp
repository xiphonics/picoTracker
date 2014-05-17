
#include "JackMidiDevice.h"
#include <jack/midiport.h>

JackMidiDevice::JackMidiDevice(jack_client_t *client,const char *portname):MidiDevice("Jack Midi") {
	client_=client ;
	port_=0 ;
	portName_=portname ;
	port_ = jack_port_register (client_, portName_.c_str(), JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
}

JackMidiDevice::~JackMidiDevice() {
} ;

bool JackMidiDevice::Init() {
	return(port_!=0) ;
} ;

void JackMidiDevice::Close() {
} ;

bool JackMidiDevice::Start() {
	frameOffset_=0 ;
	return true ;
} ;

void JackMidiDevice::Stop() {
} ;

void JackMidiDevice::SetFrameOffset(int offset) {
	frameOffset_=offset ;
} ;

void JackMidiDevice::OnNewFrame() {
	if (!port_) return ;
	jack_nframes_t  nframes = jack_get_buffer_size(client_);
	void* port_buf = jack_port_get_buffer(port_, nframes);
	jack_midi_clear_buffer(port_buf);
} ;

void JackMidiDevice::SendMessage(MidiMessage &msg) {
	jack_nframes_t nframes = jack_get_buffer_size(client_);
	void* port_buf = jack_port_get_buffer(port_, nframes);
	unsigned char *buffer = jack_midi_event_reserve(port_buf, frameOffset_, 3);
	buffer[0] = msg.status_ ;
	buffer[1] = msg.data1_ ;
	buffer[2] = msg.data2_ ;
} ;