#ifndef _MIDIIN_DEVICE_H_
#define _MIDIIN_DEVICE_H_

#include "Services/Controllers/ControllerSource.h"
#include "Foundation/T_Stack.h"
#include "Foundation/Observable.h"
#include "MidiMessage.h"
#include "MidiChannel.h"

enum MidiSyncMessage {
	MSM_START,
	MSM_STOP,
	MSM_TEMPOTICK
} ;

struct MidiSyncData:public I_ObservableData {
	MidiSyncMessage message_ ;
	MidiSyncData(MidiSyncMessage msg):message_(msg) {} ;
} ;

class MidiInDevice: public Observable,public ControllerSource,protected T_Stack<MidiMessage> {
public:
	MidiInDevice(const char *name) ;
	virtual ~MidiInDevice() ;
	bool Init() ;
	void Close() ;
	bool Start() ;
	void Stop() ;

	virtual Channel *GetChannel(const char *name) ;
	virtual bool IsRunning() ;
	virtual void Trigger(Time time) ;

protected:
	// Driver specific initialisation
	virtual bool initDriver()=0 ;
	virtual void closeDriver()=0 ;
	virtual bool startDriver()=0 ;
	virtual void stopDriver()=0 ;

	void treatChannelEvent(MidiMessage &event) ;
	void treatCC(MidiChannel *channel,int,bool hiNibble=false) ;
	void treatNoteOff(MidiChannel *channel) ;
	void treatNoteOn(MidiChannel *channel,int value) ;
	bool isRunning_ ;

	// Callbacks from driver

	void onDriverMessage(MidiMessage &event) ;
/*	void onMidiTempoTick() ;
	void onMidiStart() ;
	void onMidiStop() ;
	void queueEvent(MidiEvent &event) ;
*/
private:
  static bool dumpEvents_;
	// MIDI Channel dependant channels
	MidiChannel *ccChannel_[16][128] ;		// Control Change
	MidiChannel *noteChannel_[16][128] ;	// Note on / note off
	MidiChannel *atChannel_[16][128] ;		// After touch
	MidiChannel *pbChannel_[16] ;			// Pitch bend
	MidiChannel *catChannel_[16] ;			// Channel after touch
	MidiChannel *pcChannel_[16] ;			// Program change
} ;

#endif