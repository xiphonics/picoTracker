/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _INSTRUMENT_BANK_H_
#define _INSTRUMENT_BANK_H_

#include "Application/Instruments/I_Instrument.h"
#include "Application/Model/Song.h"
#include "Application/Persistency/Persistent.h"
#include "ChiptuneInstrument/ChiptuneInstrument.h"
#include "Externals/etl/include/etl/variant_pool.h"
#include "MidiInstrument.h"
#include "NoneInstrument.h"
#include "OpalInstrument.h"
#include "SIDInstrument.h"
#include "SampleInstrument.h"

#define NO_MORE_INSTRUMENT 0x100

enum class InstrumentAssignResult {
  Success,
  PoolExhausted,
  InitFailed,
};

class InstrumentBank : public Persistent {
public:
  InstrumentBank();
  ~InstrumentBank();
  void Reset();
  void AssignDefaults();
  I_Instrument *GetInstrument(int i);
  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);
  void Init();
  void OnStart();
  InstrumentAssignResult AssignInstrumentToSlot(InstrumentType type,
                                                unsigned char id);
  void releaseInstrument(unsigned short id);
  unsigned short Clone(unsigned short i);
  unsigned short GetNextFreeInstrumentSlotId();
  uint32_t UsedInstrumentCount() const;

  const etl::array<I_Instrument *, MAX_INSTRUMENT_COUNT> &
  InstrumentsList() const {
    return instruments_;
  }

private:
  etl::array<I_Instrument *, MAX_INSTRUMENT_COUNT> instruments_;
  etl::variant_pool<MAX_INSTRUMENT_COUNT, SampleInstrument, SIDInstrument,
                    OpalInstrument, MidiInstrument, ChiptuneInstrument>
      instrumentPool_;
  NoneInstrument none_ = NoneInstrument();
  unsigned short sidOscCount = 0;
  void purgeInstrument(I_Instrument *instrument);
};

#endif
