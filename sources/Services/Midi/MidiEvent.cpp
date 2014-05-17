
#include "MidiEvent.h"
#include "System/System/System.h"
#include <memory.h>

MidiEvent::MidiEvent(unsigned char status,unsigned char data1,unsigned char data2)
{
	type_=MET_SHORT ;
	status_=status ;
	data1_=data1 ;
	data2_=data2 ;
	buffer_=0 ;
} ;

MidiEvent::MidiEvent(char *buffer,int len) 
{
	type_=MET_SYSEX ;
	buffer_=(unsigned char *)malloc(len) ;
	bufferSize_=len ;
	memcpy(buffer_,buffer,len) ;
} ;
