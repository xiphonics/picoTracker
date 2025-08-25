/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADVMIDISERVICE_H_
#define _ADVMIDISERVICE_H_

#include "Services/Midi/MidiService.h"
#include "advMidiInDevice.h"
#include "advMidiOutDevice.h"
#include "advUSBMidiOutDevice.h"

class advMidiService : public MidiService {
public:
  advMidiService();
  ~advMidiService();

  // Poll MIDI input devices for new messages
  void poll();

private:
  advMidiOutDevice midiOutDevice_;
  advUSBMidiOutDevice usbMidiOutDevice_;
  advMidiInDevice midiInDevice_;
  // advUSBMidiInDevice usbMidiInDevice_;
};

#endif
