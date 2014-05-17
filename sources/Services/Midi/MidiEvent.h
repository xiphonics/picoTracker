
#ifndef _MIDI_EVENT_H_
#define _MIDI_EVENT_H_

enum MidiEventType {
	MET_SHORT,
	MET_SYSEX
} ;

class MidiEvent {
public:
	MidiEvent(unsigned char status = 0,unsigned char data1 = 0,unsigned char data2 = 0) ;
	MidiEvent(char *buffer,int len) ;

	MidiEventType type_ ;
	unsigned char status_ ;
	unsigned char data1_ ;
	unsigned char data2_ ;
	unsigned char *buffer_ ;
	int bufferSize_ ;
} ;

#endif