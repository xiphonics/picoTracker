
/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

/* TODOS:
    - use SampleRate::sampleRate
    - reduce RAM footprint
*/

#pragma once

#include "Application/Instruments/I_Instrument.h"
#include "Application/Model/Song.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "ChiptuneEngine.h"
#include "System/Console/Trace.h"
#include <cstdint>

class ChiptuneInstrument : public I_Instrument {

public:
  ChiptuneInstrument();
  virtual ~ChiptuneInstrument(){};

  virtual bool Init() { return true; }
  virtual bool IsInitialized() { return true; };
  virtual bool IsEmpty() { return false; };

  virtual bool SupportsCommand(FourCC cc);

  virtual InstrumentType GetType() { return IT_CHIPTUNE; };

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note, bool retrigger = true);
  virtual void Stop(int channel);

  virtual void OnStart(){};
  virtual void Purge(){};

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);

  virtual int GetTable() { return vTable_.GetInt(); };
  virtual bool GetTableAutomation() { return 0; };
  virtual void GetTableState(TableSaveState &state){};
  virtual void SetTableState(TableSaveState &state){};
  etl::ilist<Variable *> *Variables() { return &variables_; };

  void setChannel(uint8_t channel);

private:
  static voice_t voices_[SONG_CHANNEL_COUNT];

  etl::list<Variable *, 13> variables_;

  Variable vArpSpeed_;
  Variable vAttack_;
  Variable vBurst_;
  Variable vDecay_;
  Variable vLength_;
  Variable vLevel_;
  Variable vSweepAmount_;
  Variable vSweepTime_;
  Variable vTable_;
  Variable vTranspose_;
  Variable vVibratoDelay_;
  Variable vVibratoDepth_;
  Variable vWaveform_;

  void RunCommand(int channel);
  void CommandInitArp(int channel, ushort value);
  InstrumentParameters getInstrumentParameters();
};