/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _ADV_MIDI_IN_DEVICE_H_
#define _ADV_MIDI_IN_DEVICE_H_

#include "Services/Midi/MidiInDevice.h"

class advMidiInDevice : public MidiInDevice {
public:
  advMidiInDevice(const char *name);
  virtual ~advMidiInDevice();

  // Poll the MIDI input buffer for new messages
  virtual void poll();

  virtual bool Start();
  virtual void Stop();

protected:
  // Driver specific implementation
  virtual bool initDriver();
  virtual void closeDriver();
  virtual bool startDriver();
  virtual void stopDriver();
};
#endif
