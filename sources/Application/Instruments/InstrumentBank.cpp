
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

InstrumentBank::InstrumentBank()
    : Persistent("INSTRUMENTBANK"), si0(), si1(), si2(), si3(), si4(), si5(),
      si6(), si7(), si8(), si9(), si10(), si11(), si12(), si13(), si14(),
      si15(), mi0(), mi1(), mi2(), mi3(), mi4(), mi5(), mi6(), mi7(), mi8(),
      mi9(), mi10(), mi11(), mi12(), mi13(), mi14(), mi15() {

  instrument_[0] = &si0;
  instrument_[1] = &si1;
  instrument_[2] = &si2;
  instrument_[3] = &si3;
  instrument_[4] = &si4;
  instrument_[5] = &si5;
  instrument_[6] = &si6;
  instrument_[7] = &si7;
  instrument_[8] = &si8;
  instrument_[9] = &si9;
  instrument_[10] = &si10;
  instrument_[11] = &si11;
  instrument_[12] = &si12;
  instrument_[13] = &si13;
  instrument_[14] = &si14;
  instrument_[15] = &si15;
  mi0.SetChannel(0);
  instrument_[16] = &mi0;
  mi1.SetChannel(1);
  instrument_[17] = &mi1;
  mi2.SetChannel(2);
  instrument_[18] = &mi2;
  mi3.SetChannel(3);
  instrument_[19] = &mi3;
  mi4.SetChannel(4);
  instrument_[20] = &mi4;
  mi5.SetChannel(5);
  instrument_[21] = &mi5;
  mi6.SetChannel(6);
  instrument_[22] = &mi6;
  mi7.SetChannel(7);
  instrument_[23] = &mi7;
  mi8.SetChannel(8);
  instrument_[24] = &mi8;
  mi9.SetChannel(9);
  instrument_[25] = &mi9;
  mi10.SetChannel(10);
  instrument_[26] = &mi10;
  mi11.SetChannel(11);
  instrument_[27] = &mi11;
  mi12.SetChannel(12);
  instrument_[28] = &mi12;
  mi13.SetChannel(13);
  instrument_[29] = &mi13;
  mi14.SetChannel(14);
  instrument_[30] = &mi14;
  mi15.SetChannel(15);
  instrument_[31] = &mi15;

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

      for (auto it = instr->begin(); it != instr->end(); it++) {
        printer->OpenElement("PARAM");
        printer->PushAttribute("NAME", (*it)->GetName());
        printer->PushAttribute("VALUE", (*it)->GetString().c_str());
        printer->CloseElement(); // PARAM
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

          for (auto it = instr->begin(); it != instr->end(); it++) {
            if (!strcmp((*it)->GetName(), name)) {
              (*it)->SetString(value);
            };
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
  unsigned short next = GetNext();

  if (next == NO_MORE_INSTRUMENT) {
    return NO_MORE_INSTRUMENT;
  }

  I_Instrument *src = instrument_[i];
  I_Instrument *dst = instrument_[next];

  if (src == dst) {
    return NO_MORE_INSTRUMENT;
  }

  if (src->GetType() == IT_SAMPLE) {
    dst = new SampleInstrument();
  } else {
    dst = new MidiInstrument();
  }
  instrument_[next] = dst;
  for (auto it = src->begin(); it != src->end(); it++) {
    Variable *dstV = dst->FindVariable((*it)->GetID());
    if (dstV) {
      dstV->CopyFrom(**it);
    }
  }
  return next;
}

void InstrumentBank::OnStart() {
  for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {
    instrument_[i]->OnStart();
  }
  init_filters();
};
