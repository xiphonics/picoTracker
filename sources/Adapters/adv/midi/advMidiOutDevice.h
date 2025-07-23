/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADVMIDIDEVICE_H_
#define _ADVMIDIDEVICE_H_

#include "Services/Midi/MidiOutDevice.h"

class advMidiOutDevice : public MidiOutDevice {
public:
  advMidiOutDevice(const char *name);
  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

protected:
  virtual void SendMessage(MidiMessage &);

private:
};
#endif
