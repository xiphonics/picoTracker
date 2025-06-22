/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PICOTRACKERUSBMIDIDEVICE_H_
#define _PICOTRACKERUSBMIDIDEVICE_H_

#include "Services/Midi/MidiOutDevice.h"

class picoTrackerUSBMidiOutDevice : public MidiOutDevice {
public:
  picoTrackerUSBMidiOutDevice(const char *name);
  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

protected:
  virtual void SendMessage(MidiMessage &);

private:
};
#endif