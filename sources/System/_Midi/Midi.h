
#ifndef _MIDI_H_
#define _MIDI_H_

#include "Foundation/T_Factory.h"
#include "Foundation/T_SimpleList.h"
#include "Foundation/Types/Types.h"

struct MidiMessage {
    MidiMessage(uchar status=0,uchar data1=0,uchar data2=0):status_(status),data1_(data1),data2_(data2) {
    }
	unsigned char status_ ;
	unsigned char data1_ ;
	unsigned char data2_ ;
} ;

#define MIDI_MAX_BUFFERS 20

class Midi: public T_Factory<Midi> {
public:
	Midi() ;
	virtual ~Midi() {} ;
	virtual bool Init() ;
	virtual void Close() ;
	virtual bool Start() ;
	virtual void Stop() ;
	virtual void SendMessage(MidiMessage &m)=0 ;

	bool Enabled() ;

	void Trigger() ;
	void QueueMessage(MidiMessage &) ;
	void Flush() ;
protected:

	bool initialized_ ;
private:
	T_SimpleList<MidiMessage> *queues_[MIDI_MAX_BUFFERS] ;
	int currentPlayQueue_ ;
	int currentOutQueue_ ;
	bool playing_ ;
} ;
#endif
