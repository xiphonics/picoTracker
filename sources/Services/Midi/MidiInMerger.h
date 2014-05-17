#ifndef _MIDI_IN_MERGER_H_
#define _MIDI_IN_MERGER_H_

#include "MidiChannel.h"
#include "MidiInDevice.h"

#include <string>
#include <map>

typedef std::map<std::string,Channel *> tChannelMap ;

class MidiInMerger: public Observable, public ControllerSource,public T_SimpleList<MidiInDevice> {
public:
	MidiInMerger() ;
	~MidiInMerger() ;
	virtual bool IsRunning() { return true ; } ;
protected:
	virtual Channel *GetChannel(const char *name) ;
private:
	tChannelMap channels_ ;

} ;
#endif