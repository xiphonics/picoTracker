
#ifndef _MIDI_SERVICE_H_
#define _MIDI_SERVICE_H_

#include "Externals/etl/include/etl/vector.h"
#include "Foundation/Observable.h"
#include "Foundation/T_Factory.h"
#include "MidiInDevice.h"
#include "MidiInMerger.h"
#include "MidiOutDevice.h"
#include "System/Timer/Timer.h"
#include <string>

#define MIDI_MAX_BUFFERS 20

class MidiService : public T_Factory<MidiService>,
                    public T_SimpleList<MidiOutDevice>,
                    public I_Observer {

public:
  MidiService();
  virtual ~MidiService();

  bool Init();
  void Close();
  bool Start();
  void Stop();

  void SelectDevice(const std::string &name);

  // in iterator
  // TODO: refactor this
  void InBegin() { inList_.Begin(); };
  void InNext() { inList_.Next(); };
  bool InIsDone() { return inList_.IsDone(); };
  MidiInDevice &InCurrentItem() { return inList_.CurrentItem(); };

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

protected:
  T_SimpleList<MidiInDevice> inList_;

  virtual void Update(Observable &o, I_ObservableData *d);
  void onAudioTick();

  //! start the selected midi device

  void startDevice();

  //! stop the selected midi device

  void stopDevice();

  //! build the list of available drivers

  virtual void buildDriverList() = 0;

private:
  void flushOutQueue();

private:
  std::string deviceName_;
  MidiOutDevice *device_;

  etl::vector<etl::vector<MidiMessage, 10>, MIDI_MAX_BUFFERS> queues_;
  int currentPlayQueue_;
  int currentOutQueue_;

  MidiInMerger *merger_;
  int midiDelay_;
  int tickToFlush_;
  bool sendSync_;
};
#endif
