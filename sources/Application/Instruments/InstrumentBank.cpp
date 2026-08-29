/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "InstrumentBank.h"
#include "Application/Instruments/MidiInstrument.h"
#include "Application/Instruments/SIDInstrument.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Utils/char.h"
#include "ChiptuneInstrument.h"
#include "Filters.h"
#include "MidiInstrument.h"
#include "OpalInstrument.h"
#include "SIDInstrument.h"
#include "System/io/Status.h"

#define XML_DEBUG_LOGGING 0

// Contain all instrument definition
InstrumentBank::InstrumentBank()
    : Persistent("INSTRUMENTBANK"), instrumentPool_() {

  for (size_t i = 0; i < instruments_.max_size(); i++) {
    instruments_[i] = &none_;
  }

  Status::Set("All instruments preloaded");
};

InstrumentBank::~InstrumentBank() { Reset(); };

void InstrumentBank::Reset() {
  instrumentPool_.release_all();

  for (size_t i = 0; i < instruments_.max_size(); i++) {
    instruments_[i] = &none_;
  }
  sidOscCount = 0;
};

I_Instrument *InstrumentBank::GetInstrument(int i) { return instruments_[i]; };

void InstrumentBank::SaveContent(tinyxml2::XMLPrinter *printer) {
  char hex[3];
  int i = 0;
  for (auto &instr : instruments_) {
    if (!instr->IsEmpty()) {
      hex2char(i, hex);
      printer->OpenElement("INSTRUMENT");
      printer->PushAttribute("ID", hex);

      // Let the instrument save its own content
      instr->SaveContent(printer);

      printer->CloseElement(); // INSTRUMENT
    }
    i++;
  }
};

void InstrumentBank::RestoreContent(PersistencyDocument *doc) {

  bool elem = doc->FirstChild();
  while (elem) {
    // Check it is an instrument
    if (!strcasecmp(doc->ElemName(), "INSTRUMENT")) {
      // Get the instrument ID
      unsigned char id = '\0';
      char instype[16];
      instype[0] = '\0';
      bool hasId = false;
      bool hasType = false;
      bool hasAttr = doc->NextAttribute();
      while (hasAttr) {
        if (!strcasecmp(doc->attrname_, "ID")) {
          unsigned char b1 = (c2h__(doc->attrval_[0])) << 4;
          unsigned char b2 = c2h__(doc->attrval_[1]);
          id = b1 + b2;
          hasId = true;
#if XML_DEBUG_LOGGING
          Trace::Log("INSTRUMENTBANK", "instrument ID from xml:%d", id);
#endif
        }
        if (!strcasecmp(doc->attrname_, "TYPE")) {
          strncpy(instype, doc->attrval_, sizeof(instype) - 1);
          instype[sizeof(instype) - 1] = '\0';
          hasType = true;
#if XML_DEBUG_LOGGING
          Trace::Log("INSTRUMENTBANK", "instrument type from xml:%s", instype);
#endif
        }
        if (hasId && hasType) {
          break;
        }
        hasAttr = doc->NextAttribute();
      }

      InstrumentType instrType = IT_SAMPLE; // default if no type in project XML
      if (instype[0] != '\0') {
        for (uint i = 0; i < IT_LAST; i++) {
          if (!strcasecmp(instype, InstrumentTypeNames[i])) {
            instrType = (InstrumentType)i;
            break;
          }
        }
      }
      if (id < MAX_INSTRUMENT_COUNT) {
        if (AssignInstrumentToSlot(instrType, id) !=
            InstrumentAssignResult::Success) {
          Trace::Error("Failed to allocate instrument type:%d", instrType);
          // TODO: need to show user error message that proj file is invalid
        }
        I_Instrument *instr = instruments_[id];

        // Let the instrument restore its own content
        instr->RestoreContent(doc);
      }
    }
    elem = doc->NextSibling();
  };
};

void InstrumentBank::Init() {}

// Get the next available instance of the given Instrument type from the pool of
// unused Instruments and assign it to the given instrument "slot id"
InstrumentAssignResult
InstrumentBank::AssignInstrumentToSlot(InstrumentType type, uint8_t id) {
  I_Instrument *current = nullptr;

  switch (type) {
  case IT_SAMPLE:
    current = instrumentPool_.create<SampleInstrument>();
    break;
  case IT_MIDI:
    current = instrumentPool_.create<MidiInstrument>();
    break;
  case IT_SID:
    // TODO need to figure out how to properly manage sid oc count
    current = instrumentPool_.create<SIDInstrument>(SID1);
    break;
  case IT_OPAL:
    current = instrumentPool_.create<OpalInstrument>();
    break;
  case IT_CHIPTUNE:
    current = instrumentPool_.create<ChiptuneInstrument>();
    break;
  case IT_NONE:
    instruments_[id] = &none_;
    return InstrumentAssignResult::Success;
  default:
    break;
  }

  if (current == nullptr) {
    Trace::Error("Instrument pool exhausted");
    return InstrumentAssignResult::PoolExhausted;
  }

  if (!current->Init()) {
    Trace::Error("Failed to initialize new %s instrument of type",
                 InstrumentTypeNames[type]);
    purgeInstrument(current);
    return InstrumentAssignResult::InitFailed;
  }

  instruments_[id] = current;
  return InstrumentAssignResult::Success;
};

void InstrumentBank::purgeInstrument(I_Instrument *instrument) {
  switch (instrument->GetType()) {
  case IT_SAMPLE:
    instrumentPool_.destroy(static_cast<SampleInstrument *>(instrument));
    break;
  case IT_MIDI:
    instrumentPool_.destroy(static_cast<MidiInstrument *>(instrument));
    break;
  case IT_SID:
    instrumentPool_.destroy(static_cast<SIDInstrument *>(instrument));
    break;
  case IT_OPAL:
    instrumentPool_.destroy(static_cast<OpalInstrument *>(instrument));
    break;
  case IT_CHIPTUNE:
    instrumentPool_.destroy(static_cast<ChiptuneInstrument *>(instrument));
    break;
  case IT_NONE:
    // NA: None is a "singleton" so no need to release from pool
    // BUT it can be assigned to any number of slots
  default:
    break;
  }
};

void InstrumentBank::releaseInstrument(unsigned short id) {
  auto instrument = instruments_[id];

  purgeInstrument(instrument);
  instruments_[id] = &none_;
}

unsigned short InstrumentBank::GetNextFreeInstrumentSlotId() {
  for (unsigned short i = 0; i < instruments_.max_size(); i++) {
    if (instruments_[i] == &none_) {
      return i;
    }
  }
  return NO_MORE_INSTRUMENT;
}

unsigned short InstrumentBank::Clone(unsigned short i) {
  I_Instrument *src = instruments_[i];

  // Find next available instrument slot
  auto nextFreeInstrumentSlotId = GetNextFreeInstrumentSlotId();

  if (nextFreeInstrumentSlotId == NO_MORE_INSTRUMENT) {
    return NO_MORE_INSTRUMENT;
  }

  InstrumentAssignResult result =
      AssignInstrumentToSlot(src->GetType(), nextFreeInstrumentSlotId);

  if (result != InstrumentAssignResult::Success) {
    return NO_MORE_INSTRUMENT;
  }

  I_Instrument *dst = instruments_[nextFreeInstrumentSlotId];

  // sanity check not trying to clone into itself
  if (src == dst) {
    return NO_MORE_INSTRUMENT;
  }

  for (auto it = src->Variables()->begin(); it != src->Variables()->end();
       it++) {
    Variable *dstV = dst->FindVariable((*it)->GetID());
    if (dstV) {
      dstV->CopyFrom(**it);
    }
  }
  return nextFreeInstrumentSlotId;
}

void InstrumentBank::OnStart() {
  for (auto &elem : instruments_) {
    elem->OnStart();
  }
  init_filters();
};

uint32_t InstrumentBank::UsedInstrumentCount() const {
  uint32_t count = 0;
  for (auto &elem : instruments_) {
    if (!elem->IsEmpty()) {
      count++;
    }
  }
  return count;
}
