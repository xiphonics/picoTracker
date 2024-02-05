
#include "InstrumentBank.h"
#include "Application/Instruments/MidiInstrument.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Utils/char.h"
#include "Filters.h"
#include "System/io/Status.h"

const char *InstrumentTypeData[] = {"Sample", "Midi"};

// Contain all instrument definition

InstrumentBank::InstrumentBank() : Persistent("INSTRUMENTBANK") {

  for (int i = 0; i < MAX_SAMPLEINSTRUMENT_COUNT; i++) {
    Trace::Debug("Loading sample instrument: %i", i);
    SampleInstrument *s = new SampleInstrument();
    instrument_[i] = s;
  }
  for (int i = 0; i < MAX_MIDIINSTRUMENT_COUNT; i++) {
    Trace::Debug("Loading MIDI instrument: %i", i);
    MidiInstrument *s = new MidiInstrument();
    s->SetChannel(i);
    instrument_[MAX_SAMPLEINSTRUMENT_COUNT + i] = s;
  }
  Status::Set("All instrument loaded");
};

//
// Assigns default instruments value for new project
//

void InstrumentBank::AssignDefaults() {

  SamplePool *pool = SamplePool::GetInstance();
  for (int i = 0; i < MAX_SAMPLEINSTRUMENT_COUNT; i++) {
    SampleInstrument *s = (SampleInstrument *)instrument_[i];
    if (i < pool->GetNameListSize()) {
      s->AssignSample(i);
    } else {
      s->AssignSample(-1);
    }
  };
};

InstrumentBank::~InstrumentBank() {
  for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {
    delete instrument_[i];
  }
};

I_Instrument *InstrumentBank::GetInstrument(int i) { return instrument_[i]; };

void InstrumentBank::SaveContent(tinyxml2::XMLPrinter *printer) {
  char hex[3];
  for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {

    I_Instrument *instr = instrument_[i];
    if (!instr->IsEmpty()) {
      printer->OpenElement("INSTRUMENT");
      hex2char(i, hex);
      printer->PushAttribute("ID", hex);
      printer->PushAttribute("TYPE", InstrumentTypeData[instr->GetType()]);

      auto it = instr->begin();
      for (size_t j = 0; j < instr->size(); j++) {
        printer->OpenElement("PARAM");
        printer->PushAttribute("NAME", (*it)->GetName());
        printer->PushAttribute("VALUE", (*it)->GetString());
        printer->CloseElement(); // PARAM
        it++;
      }
      printer->CloseElement(); // INSTRUMENT
    }
  }
};

void InstrumentBank::RestoreContent(PersistencyDocument *doc) {

  if (doc->version_ < 130) {
    if (Config::GetInstance()->GetValue("LEGACYDOWNSAMPLING") != NULL) {
      SampleInstrument::EnableDownsamplingLegacy();
    }
  }
  bool elem = doc->FirstChild();
  while (elem) {
    // Check it is an instrument
    if (!strcmp(doc->ElemName(), "INSTRUMENT")) {
      // Get the instrument ID
      unsigned char id = '\0';
      char *instype = NULL;
      bool hasAttr = doc->NextAttribute();
      while (hasAttr) {
        if (!strcmp(doc->attrname_, "ID")) {
          unsigned char b1 = (c2h__(doc->attrval_[0])) << 4;
          unsigned char b2 = c2h__(doc->attrval_[1]);
          id = b1 + b2;
        }
        if (!strcmp(doc->attrname_, "TYPE")) {
          instype = doc->attrval_;
        }
        hasAttr = doc->NextAttribute();
      }

      InstrumentType it;
      if (instype) {
        for (uint i = 0; i < sizeof(InstrumentTypeData); i++) {
          if (!strcmp(instype, InstrumentTypeData[i])) {
            it = (InstrumentType)i;
            break;
          }
        }
      } else {
        it = (id < MAX_SAMPLEINSTRUMENT_COUNT) ? IT_SAMPLE : IT_MIDI;
      };
      if (id < MAX_INSTRUMENT_COUNT) {
        I_Instrument *instr = instrument_[id];
        if (instr->GetType() != it) {
          delete instr;
          switch (it) {
          case IT_SAMPLE:
            instr = new SampleInstrument();
            break;
          case IT_MIDI:
            instr = new MidiInstrument();
            break;
          }
          instrument_[id] = instr;
        };

        bool subelem = doc->FirstChild();
        while (subelem) {
          bool hasAttr = doc->NextAttribute();
          char name[24];
          char value[24];
          while (hasAttr) {
            if (!strcmp(doc->attrname_, "NAME")) {
              strcpy(name, doc->attrval_);
            }
            if (!strcmp(doc->attrname_, "VALUE")) {
              strcpy(value, doc->attrval_);
            }
            hasAttr = doc->NextAttribute();
          }

          // Convert old filter dist to newer filter mode
          if (!strcmp(name, "filter dist")) {
            strcpy(name, "filter mode");
            if (!strcmp(value, "none")) {
              strcpy(value, "original");
            } else {
              strcpy(value, "scream");
            }
          }

          auto it = instr->begin();
          for (size_t i = 0; i < instr->size(); i++) {
            if (!strcmp((*it)->GetName(), name)) {
              (*it)->SetString(value);
            };
            it++;
          }
          subelem = doc->NextSibling();
        }
        if (doc->version_ < 38) {
          Variable *cvl = instr->FindVariable(SIP_CRUSHVOL);
          Variable *vol = instr->FindVariable(SIP_VOLUME);
          Variable *crs = instr->FindVariable(SIP_CRUSH);
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

void InstrumentBank::Init() {
  for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {
    instrument_[i]->Init();
  }
}

unsigned short InstrumentBank::GetNext() {
  for (int i = 0; i < MAX_SAMPLEINSTRUMENT_COUNT; i++) {
    SampleInstrument *si = (SampleInstrument *)instrument_[i];
    Variable *sample = si->FindVariable(SIP_SAMPLE);
    if (sample) {
      if (sample->GetInt() == -1) {
        return i;
      }
    }
  }
  return NO_MORE_INSTRUMENT;
};

unsigned short InstrumentBank::Clone(unsigned short i) {
  // can't clone midi instruments

  unsigned short next = GetNext();
  if (next == NO_MORE_INSTRUMENT) {
    return NO_MORE_INSTRUMENT;
  }

  I_Instrument *src = instrument_[i];
  I_Instrument *dst = instrument_[next];

  if (src == dst) {
    return NO_MORE_INSTRUMENT;
  }

  delete dst;

  if (src->GetType() == IT_SAMPLE) {
    dst = new SampleInstrument();
  } else {
    dst = new MidiInstrument();
  }
  instrument_[next] = dst;
  auto it = src->begin();
  for (size_t i = 0; i < src->size(); i++) {
    Variable *dstV = dst->FindVariable((*it)->GetID());
    if (dstV) {
      dstV->CopyFrom(**it);
    }
    it++;
  }
  return next;
}

void InstrumentBank::OnStart() {
  for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {
    instrument_[i]->OnStart();
  }
  init_filters();
};
