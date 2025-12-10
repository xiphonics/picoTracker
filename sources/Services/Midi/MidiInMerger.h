/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MIDI_IN_MERGER_H_
#define _MIDI_IN_MERGER_H_

#include "Externals/etl/include/etl/map.h"
#include "Externals/etl/include/etl/string.h"
#include "MidiChannel.h"
#include "MidiInDevice.h"
#include "config/StringLimits.h"

// Maximum number of merged MIDI channels we expect to track
static constexpr size_t MIDI_MERGER_MAX_CHANNELS = 16;

typedef etl::map<etl::string<STRING_MIDI_IN_KEY_MAX>, Channel *,
                 MIDI_MERGER_MAX_CHANNELS>
    tChannelMap;

class MidiInMerger : public Observable,
                     public ControllerSource,
                     public T_SimpleList<MidiInDevice> {
public:
  MidiInMerger();
  ~MidiInMerger();
  virtual bool IsRunning() { return true; };

protected:
  virtual Channel *GetChannel(const char *name);

private:
  tChannelMap channels_;
};
#endif
