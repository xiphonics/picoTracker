/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 * Copyright (c) 2925 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "Application/Model/Song.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "GameBoyEngine.h"
#include "I_Instrument.h"
#include "System/Console/Trace.h"
#include <cstdint>

class GameBoyInstrument : public I_Instrument {

public:
  GameBoyInstrument();
  virtual ~GameBoyInstrument(){};

  virtual bool Init() { return true; }
  virtual bool IsInitialized() { return true; };
  virtual bool IsEmpty() { return false; };

  virtual bool SupportsCommand(FourCC cc);

  virtual InstrumentType GetType() { return IT_GAMEBOY; };

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

  Variable vWaveform_;
  Variable vAttack_;
  Variable vDecay_;
  Variable vLevel_;
  Variable vLength_;
  Variable vBurst_;
  Variable vVibratoDepth_;
  Variable vVibratoDelay_;
  Variable vTranspose_;
  Variable vTable_;
  Variable vArpSpeed_;
  Variable vSweepTime_;
  Variable vSweepAmount_;

  void RunCommand(int channel);
  void CommandInitArp(int channel, ushort value);
  InstrumentParameters getInstrumentParameters();
};