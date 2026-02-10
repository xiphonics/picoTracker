/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _OPAL_INSTRUMENT_H_
#define _OPAL_INSTRUMENT_H_

#include "Application/Model/Song.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "Externals/opal/opal.h"
#include "I_Instrument.h"
#include <cstdint>

#define OPAL_MAX_CHANNELS 4

class OpalInstrument : public I_Instrument {

public:
  OpalInstrument();
  virtual ~OpalInstrument();

  virtual bool Init();

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note, bool retrigger = true);
  virtual void Stop(int channel);

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual bool SupportsCommand(FourCC cc) override;
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);

  virtual bool IsInitialized();

  virtual bool IsEmpty() { return false; };

  virtual InstrumentType GetType() { return IT_OPAL; };

  virtual void OnStart();

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };

  void setChannel(uint8_t channel);

private:
  Opal opl_ = (44100);

  uint8_t breg;

  etl::list<Variable *, 16> variables_;

  Variable algorithm_;
  Variable feedback_;
  Variable deepTremeloVibrato_;

  Variable op1Level_;
  Variable op1Multiplier_;
  Variable op1ADSR_;
  Variable op1WaveShape_;
  // Termelo(AM),Vibrato(VIB),SustainingVoice(EG),EnveloperScale(KSR)
  Variable op1TremVibSusKSR_;
  Variable op1KeyScaleLevel_;
  Variable op2Level_;
  Variable op2Multiplier_;
  Variable op2ADSR_;
  Variable op2WaveShape_;
  // Termelo(AM),Vibrato(VIB),SustainingVoice(EG),EnveloperScale(KSR)
  Variable op2TremVibSusKSR_;
  Variable op2KeyScaleLevel_;
};

#endif
