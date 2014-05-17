#include "Midi.h"
#include "System/io/Trace.h"
#include "Application/Player/SyncMaster.h"

Midi::Midi() {
	for (int i=0;i<MIDI_MAX_BUFFERS;i++) {
		queues_[i]=new T_SimpleList<MidiMessage>(true)  ;
	}
	playing_=false ;
    initialized_=false ;
} ;

bool Midi::Init() {
    Trace::Dump("Midi::Init()") ;
	currentPlayQueue_=0 ;
	currentOutQueue_=0 ;
    initialized_=true ;
    return true ;    
}
void Midi::Close() {
    Trace::Dump("Midi::Close()") ;
    if (playing_) {
        Stop() ;
    } ;
    initialized_=false ;
}

bool Midi::Enabled() {
    return initialized_ ;
}

bool Midi::Start() {
	playing_=true ;
	MidiMessage msg ;
	msg.status_=0xFA ;
	QueueMessage(msg) ;
	return true ;
} ;

void Midi::Stop() {
	if (playing_) {
		MidiMessage msg ;
		msg.status_=0xFC ;
		QueueMessage(msg) ;
		QueueMessage(msg) ;
   		Midi::Flush() ;
	}
	playing_=false ;
} ;

void Midi::QueueMessage(MidiMessage &m) {
     if (playing_) {
        T_SimpleList<MidiMessage> *queue=queues_[currentPlayQueue_] ;
	    MidiMessage *ms=new MidiMessage(m.status_,m.data1_,m.data2_) ;
	    queue->Insert(ms) ;
   }
} ;

void Midi::Trigger() {
	if (Enabled()&&(playing_)) {
 		currentPlayQueue_=(currentPlayQueue_+1)%MIDI_MAX_BUFFERS ;
		T_SimpleList<MidiMessage> *queue=queues_[currentPlayQueue_] ;
		queue->Empty() ;
		SyncMaster *sm=SyncMaster::GetInstance() ;
		if (sm->MidiSlice()) {
			MidiMessage msg ;
			msg.status_=0xF8 ;
			QueueMessage(msg) ;
		};
	}
} ;

void Midi::Flush() {
//     Trace::Debug("Midi flush") ;
     if (Enabled()) {
        T_SimpleList<MidiMessage> *flushQueue=queues_[currentOutQueue_] ;
	    IteratorPtr<MidiMessage> it(flushQueue->GetIterator()) ;
	    for (it->Begin();!it->IsDone();it->Next()) {
		    MidiMessage msg=it->CurrentItem() ;
		    SendMessage(msg) ;
	     } ;
	    flushQueue->Empty() ;
        currentOutQueue_=(currentOutQueue_+1)%MIDI_MAX_BUFFERS ;
    }
} ;

