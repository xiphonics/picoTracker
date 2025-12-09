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
#include "I_Instrument.h"
#include <cstdint>

#define GB_NUM_WAVEFORMS 5

constexpr int SAMPLING_RATE = 44100;

constexpr int BASE_MIDI = 69; // A4
constexpr double BASE_FREQ = 440.0;
constexpr int PHASE_BITS = 32;

class GameBoyInstrument : public I_Instrument {

public:
  GameBoyInstrument();
  virtual ~GameBoyInstrument();

  virtual bool Init();

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note, bool retrigger = true);
  virtual void Stop(int channel);

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);

  virtual bool IsInitialized();

  virtual bool IsEmpty() { return false; };

  virtual InstrumentType GetType() { return IT_GAMEBOY; };

  virtual void OnStart();

  virtual void Purge(){};

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };

  void setChannel(uint8_t channel);

private:
  etl::list<Variable *, 12> variables_;

  Variable waveform_;
  Variable attack_;
  Variable decay_;

  Variable level_;
  Variable length_;
  Variable burst_;
  Variable vibrato_;
  Variable vibratoDelay_;
  Variable transpose_;
  Variable table_;
  Variable sweepTime_;
  Variable sweepAmount_;

  uint32_t phase_ = 0;
  uint32_t frequency_ = 0;
};