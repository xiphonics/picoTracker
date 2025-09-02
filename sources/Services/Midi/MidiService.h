#ifndef _MIDI_SERVICE_H_
#define _MIDI_SERVICE_H_

#include "Externals/etl/include/etl/vector.h"
#include "Foundation/Observable.h"
#include "Foundation/T_Factory.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "MidiInDevice.h"
#include "MidiInMerger.h"
#include "MidiOutDevice.h"
#include "System/Timer/Timer.h"
#include <string>

#define MIDI_MAX_BUFFERS 20

class MidiService : public T_Factory<MidiService>, public I_Observer {

public:
  MidiService();
  virtual ~MidiService();

  bool Init();
  void Close();
  bool Start();
  void Stop();

  //! player notification
  void OnPlayerStart();
  void OnPlayerStop();

  //! Queues a MidiMessage to the current time chunk
  void QueueMessage(MidiMessage &);

  //! Time chunk trigger
  void Trigger();
  void AdvancePlayQueue();

  //! Flush current queue to the output
  void Flush();

  //! Handle MIDI transport messages
  void OnMidiStart();
  void OnMidiStop();
  void OnMidiClock();

protected:
  etl::vector<MidiInDevice *, 2> inList_;
  etl::vector<MidiOutDevice *, 2> outList_;

  virtual void Update(Observable &o, I_ObservableData *d);
  void onAudioTick();

  // start the selected midi device
  void startDevice();

  // stop the selected midi device
  void stopDevice();

private:
  void flushOutQueue();
  void updateActiveDevicesList(unsigned short config);

private:
  etl::vector<MidiOutDevice *, 2> activeOutDevices_;

  etl::array<etl::vector<MidiMessage, MIDI_MAX_MESG_QUEUE>, MIDI_MAX_BUFFERS>
      queues_;

  int currentPlayQueue_;
  int currentOutQueue_;

  MidiInMerger *merger_;
  int midiDelay_;
  int tickToFlush_;
  bool sendSync_;
};
#endif
