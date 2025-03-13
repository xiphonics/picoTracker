
#include "InstrumentBank.h"
#include "Application/Instruments/MacroInstrument.h"
#include "Application/Instruments/MidiInstrument.h"
#include "Application/Instruments/SIDInstrument.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Utils/char.h"
#include "Filters.h"
#include "MidiInstrument.h"
#include "OpalInstrument.h"
#include "SIDInstrument.h"
#include "System/io/Status.h"

// Contain all instrument definition
InstrumentBank::InstrumentBank()
    : Persistent("INSTRUMENTBANK"), sampleInstrumentPool_(),
      midiInstrumentPool_(), sidInstrumentPool_(), opalInstrumentPool_(),
      macroInstrumentPool_() {

  for (size_t i = 0; i < instruments_.max_size(); i++) {
    instruments_[i] = &none_;
  }

  Status::Set("All instruments preloaded");
};

InstrumentBank::~InstrumentBank() {
  sampleInstrumentPool_.release_all();
  midiInstrumentPool_.release_all();
  sidInstrumentPool_.release_all();
  opalInstrumentPool_.release_all();
  macroInstrumentPool_.release_all();
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
      printer->PushAttribute("TYPE", InstrumentTypeNames[instr->GetType()]);

      for (auto it = instr->Variables()->begin();
           it != instr->Variables()->end(); it++) {
        printer->OpenElement("PARAM");
        printer->PushAttribute("NAME", (*it)->GetName());
        printer->PushAttribute("VALUE", (*it)->GetString().c_str());
        printer->CloseElement(); // PARAM
      }
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
      char *instype = NULL;
      bool hasAttr = doc->NextAttribute();
      while (hasAttr) {
        if (!strcasecmp(doc->attrname_, "ID")) {
          unsigned char b1 = (c2h__(doc->attrval_[0])) << 4;
          unsigned char b2 = c2h__(doc->attrval_[1]);
          id = b1 + b2;
          Trace::Log("INSTRUMENTBANK", "instrument ID from xml:%d", id);
        }
        if (!strcasecmp(doc->attrname_, "TYPE")) {
          instype = doc->attrval_;
          Trace::Log("INSTRUMENTBANK", "instrument type from xml:%s", instype);
        }
        hasAttr = doc->NextAttribute();
      }

      InstrumentType instrType = IT_SAMPLE; // default if no type in project XML
      if (instype) {
        for (uint i = 0; i < sizeof(InstrumentTypeNames); i++) {
          if (!strcasecmp(instype, InstrumentTypeNames[i])) {
            instrType = (InstrumentType)i;
            break;
          }
        }
      }
      if (id < MAX_INSTRUMENT_COUNT) {
        if (GetNextAndAssignID(instrType, id) == NO_MORE_INSTRUMENT) {
          Trace::Error("Failed to allocate instrument type:%d", instrType);
          // TODO: need to show user error message that proj file is invalid
        }
        I_Instrument *instr = instruments_[id];

        bool subelem = doc->FirstChild();
        while (subelem) {
          bool hasAttr = doc->NextAttribute();
          char name[24] = "";
          char value[24] = "";
          while (hasAttr) {
            if (!strcasecmp(doc->attrname_, "NAME")) {
              strcpy(name, doc->attrval_);
            }
            if (!strcasecmp(doc->attrname_, "VALUE")) {
              strcpy(value, doc->attrval_);
            }
            hasAttr = doc->NextAttribute();
          }

          // Convert old filter dist to newer filter mode
          if (!strcasecmp(name, "filter dist")) {
            strcpy(name, "filter mode");
            if (!strcasecmp(value, "none")) {
              strcpy(value, "original");
            } else {
              strcpy(value, "scream");
            }
          }

          auto vars = instr->Variables();
          for (auto elem : *vars) {
            if (!strcasecmp((elem)->GetName(), name)) {
              // Trace::Debug("Restore SetString %s->%s", name, value);
              (elem)->SetString(value);
            };
          }
          subelem = doc->NextSibling();
        }
        if (doc->version_ < 38) {
          Variable *cvl =
              instr->FindVariable(FourCC::SampleInstrumentCrushVolume);
          Variable *vol = instr->FindVariable(FourCC::SampleInstrumentVolume);
          Variable *crs = instr->FindVariable(FourCC::SampleInstrumentCrush);
          if ((vol) && (cvl) && (crs)) {
            if (crs->GetInt() != 16) {
              int temp = vol->GetInt();
              vol->SetInt(cvl->GetInt());
              cvl->SetInt(temp);
            }
          };
        }
      }
    }
    elem = doc->NextSibling();
  };
};

void InstrumentBank::Init() {}

// Get the next available instance of the given Instrument type from the pool of
// unused Instruments and assign it to the given instrument "slot id"
unsigned short InstrumentBank::GetNextAndAssignID(InstrumentType type,
                                                  uint8_t id) {
  switch (type) {
  case IT_SAMPLE: {
    SampleInstrument *si = sampleInstrumentPool_.create();
    if (si == nullptr) {
      Trace::Log("INSTRUMENTBANK", "Sample INSTRUMENT EXHAUSTED!");
    }
    // TODO: pool exhastion: show user UI message about it!
    si->Init();

    Variable *sample = si->FindVariable(FourCC::SampleInstrumentSample);
    if (sample) {
      if (sample->GetInt() == -1) {
        instruments_[id] = si;
      } else {
        Trace::Log("INSTRUMENTBANK",
                   "unexpected sample value for new instrument: %d",
                   sample->GetInt());
      }
    }
    return id;
  } break;
  case IT_MIDI: {
    MidiInstrument *mi = midiInstrumentPool_.create();
    if (mi == nullptr) {
      Trace::Error("MIDI INSTRUMENT EXHAUSTED!!!!!!");
    }
    // TODO check for pool exhastion AND show user UI message about it!!!
    mi->Init();
    instruments_[id] = mi;
    return id;
  } break;
  case IT_SID: {
    // TODO need to figure out how to properly manage sid oc count
    SIDInstrument *si = sidInstrumentPool_.create(SID1);
    if (si == nullptr) {
      Trace::Error("SID INSTRUMENT EXHAUSTED!!!!!!");
    }
    // TODO check for pool exhastion AND show user UI message about it!!!
    si->Init();
    instruments_[id] = si;
    return id;
  } break;
  case IT_OPAL: {
    OpalInstrument *oi = opalInstrumentPool_.create();
    // TODO check for pool exhastion AND show user UI message about it!!!
    if (oi == nullptr) {
      Trace::Error("Opal INSTRUMENT EXHAUSTED!!!!!!");
    }
    oi->Init();
    instruments_[id] = oi;
    return id;
  } break;
  case IT_NONE:
    instruments_[id] = &none_;
    return id;
  default:
    break;
  }

  return NO_MORE_INSTRUMENT;
};

void InstrumentBank::releaseInstrument(unsigned short id) {
  auto instrument = instruments_[id];

  switch (instrument->GetType()) {
  case IT_SAMPLE:
    sampleInstrumentPool_.destroy(instrument);
    break;
  case IT_MIDI:
    midiInstrumentPool_.destroy(instrument);
    break;
  case IT_SID:
    sidInstrumentPool_.destroy(instrument);
    break;
  case IT_OPAL:
    opalInstrumentPool_.destroy(instrument);
    break;
  case IT_NONE:
    // NA: None is a "singleton" so no need to release from pool
    // BUT it can be assigned to any number of slots
  default:
    break;
  }
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

  // TODO: NEED TO actually find the next available instrument slot, if there
  // even is one
  auto nextFreeInstrumentSlotId = i++;

  unsigned short next =
      GetNextAndAssignID(src->GetType(), nextFreeInstrumentSlotId);

  if (next == NO_MORE_INSTRUMENT) {
    return NO_MORE_INSTRUMENT;
  }

  I_Instrument *dst = instruments_[next];

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
  return next;
}

void InstrumentBank::OnStart() {
  for (auto &elem : instruments_) {
    elem->OnStart();
  }
  init_filters();
};
