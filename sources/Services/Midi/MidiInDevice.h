/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MIDIIN_DEVICE_H_
#define _MIDIIN_DEVICE_H_

#include "Foundation/Observable.h"
#include "MidiEvent.h"
#include "MidiMessage.h"
#include "MidiNoteTracker.h"

enum MidiSyncMessage { MSM_START, MSM_STOP, MSM_TEMPOTICK, MSM_CONTINUE };

struct MidiSyncData : public I_ObservableData {
  MidiSyncMessage message_;
  MidiSyncData(MidiSyncMessage msg) : message_(msg){};
};

class MidiInDevice : public Observable {
public:
  MidiInDevice(const char *name);
  virtual ~MidiInDevice();
  bool Init();
  void Close();
  virtual bool Start() = 0;
  virtual void Stop() = 0;

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
  bool isRunning_;

  // Callbacks from driver

  void onDriverMessage(MidiMessage &event);
  void onMidiTempoTick();
  void onMidiStart();
  void onMidiStop();
  void onMidiContinue();
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
