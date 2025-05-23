#ifndef _MIDIIN_DEVICE_H_
#define _MIDIIN_DEVICE_H_

#include "Foundation/Observable.h"
#include "Foundation/T_Stack.h"
#include "MidiChannel.h"
#include "MidiEvent.h"
#include "MidiMessage.h"
#include "MidiNoteTracker.h"
#include "Services/Controllers/ControllerSource.h"

enum MidiSyncMessage { MSM_START, MSM_STOP, MSM_TEMPOTICK };

struct MidiSyncData : public I_ObservableData {
  MidiSyncMessage message_;
  MidiSyncData(MidiSyncMessage msg) : message_(msg){};
};

class MidiInDevice : public Observable,
                     public ControllerSource,
                     protected T_Stack<MidiMessage> {
public:
  MidiInDevice(const char *name);
  virtual ~MidiInDevice();
  bool Init();
  void Close();
  virtual bool Start() = 0;
  virtual void Stop() = 0;

  virtual Channel *GetChannel(const char *name);
  virtual bool IsRunning();
  virtual void Trigger();

  // New methods for direct instrument mapping
  static void AssignInstrumentToChannel(int midiChannel, int instrumentIndex);
  static int GetInstrumentForChannel(int midiChannel);
  static void ClearChannelAssignment(int midiChannel);

  virtual void poll() = 0;

protected:
  // Driver specific initialisation
  virtual bool initDriver() = 0;
  virtual void closeDriver() = 0;
  virtual bool startDriver() = 0;
  virtual void stopDriver() = 0;

  void treatChannelEvent(MidiMessage &event);
  void treatCC(MidiChannel *channel, int, bool hiNibble = false);
  void treatNoteOff(MidiChannel *channel);
  void treatNoteOn(MidiChannel *channel, int value);
  bool isRunning_;

  // Callbacks from driver

  void onDriverMessage(MidiMessage &event);
  void onMidiTempoTick();
  void onMidiStart();
  void onMidiStop();
  void queueEvent(MidiEvent &event);

  // MIDI message parsing
  void processMidiData(uint8_t data);

private:
  static bool dumpEvents_;

  // New direct mapping from MIDI channels to instrument indices
  static int8_t channelToInstrument_[16];

  // MIDI message parsing state
  uint8_t midiStatus = 0;
  uint8_t midiData1 = 0;
  uint8_t midiDataCount = 0;
  uint8_t midiDataBytes = 0;

  // Note tracker for polyphonic note handling
  MidiNoteTracker noteTracker_;
};

#endif
