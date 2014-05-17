
#ifndef _MIDI_SERVICE_H_
#define _MIDI_SERVICE_H_

#include <string>
#include "Foundation/T_Factory.h"
#include "Foundation/Observable.h"
#include "System/Timer/Timer.h"
#include "MidiOutDevice.h"
#include "MidiInDevice.h"
#include "MidiInDevice.h"
#include "MidiInMerger.h"

#define MIDI_MAX_BUFFERS 20

class MidiService
:public T_Factory<MidiService>
,public T_SimpleList<MidiOutDevice>
,public I_Observer
{

public:
	MidiService() ;
	virtual ~MidiService() ;

	bool Init() ;
	void Close() ;
	bool Start() ;
	void Stop() ;

	void SelectDevice(const std::string &name) ;

	I_Iterator<MidiInDevice> *GetInIterator() ;

	//! player notification

	void OnPlayerStart() ;
	void OnPlayerStop() ;

	//! Queues a MidiMessage to the current time chunk

	void QueueMessage(MidiMessage &) ;

	//! Time chunk trigger

	void Trigger() ;
  void AdvancePlayQueue();

	//! Flush current queue to the output

	void Flush() ;
  

protected:

	T_SimpleList<MidiInDevice> inList_ ;

  virtual void Update(Observable &o,I_ObservableData *d) ;
  void onAudioTick();

	//! start the selected midi device

	void startDevice() ;

	//! stop the selected midi device

	void stopDevice() ;

	//! build the list of available drivers

	virtual void buildDriverList()=0 ;

private:
  void flushOutQueue();
private:
	std::string deviceName_ ;
	MidiOutDevice *device_ ;

	T_SimpleList<MidiMessage> *queues_[MIDI_MAX_BUFFERS] ;
	int currentPlayQueue_ ;
	int currentOutQueue_ ;

	MidiInMerger *merger_ ;
	int midiDelay_ ;
  int tickToFlush_ ;
	bool sendSync_ ;
} ;
#endif
