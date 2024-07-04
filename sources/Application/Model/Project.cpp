#include "Project.h"
#include "Application/Instruments/CommandList.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Player/SyncMaster.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "Groove.h"
#include "Scale.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "System/io/Status.h"
#include "Table.h"

#include <math.h>

Project::Project(const char *name)
    : Persistent("PROJECT"), song_(), midiDeviceList_(0), tempoNudge_(0),
      tempo_("tempo", VAR_TEMPO, 138),
      masterVolume_("master", VAR_MASTERVOL, 100),
      wrap_("wrap", VAR_WRAP, false), transpose_("transpose", VAR_TRANSPOSE, 0),
      scale_("scale", VAR_SCALE, scaleNames, numScales, 0),
      projectName_("projectname", VAR_PROJECTNAME, name) {

  //  WatchedVariable *tempo = new WatchedVariable("tempo", VAR_TEMPO, 138);
  this->insert(end(), &tempo_);
  //  Variable *masterVolume = new Variable("master", VAR_MASTERVOL, 100);
  this->insert(end(), &masterVolume_);
  //  Variable *wrap = new Variable("wrap", VAR_WRAP, false);
  this->insert(end(), &wrap_);
  //  Variable *transpose = new Variable("transpose", VAR_TRANSPOSE, 0);
  this->insert(end(), &transpose_);
  //  Variable *scale = new Variable("scale", VAR_SCALE, scaleNames, numScales,
  //  0);
  this->insert(end(), &scale_);
  scale_.SetInt(0);
  this->insert(end(), &projectName_);

  // Reload the midi device list
  buildMidiDeviceList();

  WatchedVariable *midi = new WatchedVariable(
      "midi", VAR_MIDIDEVICE, midiDeviceList_, midiDeviceListSize_);
  this->insert(end(), midi);
  midi->AddObserver(*this);

  static char instrumentBankMemBuf[sizeof(InstrumentBank)];
  instrumentBank_ = new (instrumentBankMemBuf) InstrumentBank();

  // look if we can find a sav file

  // Makes sure the tables exists for restoring

  TableHolder::GetInstance();

  Groove::GetInstance()->Clear();

  tempoTapCount_ = 0;

  Status::Set("About to load project");
};

Project::~Project() { delete instrumentBank_; };

int Project::GetScale() {
  Variable *v = FindVariable(VAR_SCALE);
  NAssert(v);
  return v->GetInt();
}

int Project::GetTempo() {
  Variable *v = FindVariable(VAR_TEMPO);
  NAssert(v);
  int tempo = v->GetInt() + tempoNudge_;
  return tempo;
};

int Project::GetMasterVolume() {
  Variable *v = FindVariable(VAR_MASTERVOL);
  NAssert(v);
  return v->GetInt();
};

void Project::GetProjectName(char *name) {
  Variable *v = FindVariable(VAR_PROJECTNAME);
  strcpy(name, v->GetString().c_str());
}

void Project::SetProjectName(char *name) {
  Variable *v = FindVariable(VAR_PROJECTNAME);
  v->SetString(name, true);
}

void Project::NudgeTempo(int value) { tempoNudge_ += value; };

void Project::Trigger() {
  if (tempoNudge_ != 0) {
    if (tempoNudge_ > 0) {
      tempoNudge_--;
    } else {
      tempoNudge_++;
    };
  }
};

int Project::GetTranspose() {
  Variable *v = FindVariable(VAR_TRANSPOSE);
  NAssert(v);
  int result = v->GetInt();
  if (result > 0x80) {
    result -= 128;
  }
  return result;
};

bool Project::Wrap() {
  Variable *v = FindVariable(VAR_WRAP);
  NAssert(v);
  return v->GetBool();
};

InstrumentBank *Project::GetInstrumentBank() { return instrumentBank_; };

// bool Project::MidiEnabled() {
//	Variable *v=FindVariable(VAR_MIDIENABLE) ;
//	NAssert(v) ;
//	return v->GetBool() ;
// }

void Project::Update(Observable &o, I_ObservableData *d) {
  WatchedVariable &v = (WatchedVariable &)o;
  switch (v.GetID()) {
  case VAR_MIDIDEVICE:
    MidiService::GetInstance()->SelectDevice(
        std::string(v.GetString().c_str()));
    /*           bool enabled=v.GetBool() ;
               Midi *midi=Midi::GetInstance() ;
               if (enabled) {
                   midi->Init() ;
               } else {
                   midi->Stop() ;
                   midi->Close() ;
               }
    */
    break;
  }
}

void Project::Purge() {

  song_.chain_.ClearAllocation();
  song_.phrase_.ClearAllocation();

  unsigned char *data = song_.data_;
  for (int i = 0; i < 256 * SONG_CHANNEL_COUNT; i++) {
    if (*data != 0xFF) {
      song_.chain_.SetUsed(*data);
    }
    data++;
  }

  data = song_.chain_.data_;
  unsigned char *data2 = song_.chain_.transpose_;

  for (int i = 0; i < CHAIN_COUNT; i++) {

    if (song_.chain_.IsUsed(i)) {
      for (int j = 0; j < 16; j++) {
        if (*data != 0xFF) {
          song_.phrase_.SetUsed(*data);
        }
        data++;
        data2++;
      }
    } else {

      for (int j = 0; j < 16; j++) {
        *data++ = 0xFF;
        *data2++ = 0x00;
      }
    }
  }

  data = song_.phrase_.note_;
  data2 = song_.phrase_.instr_;

  FourCC *cmd1 = song_.phrase_.cmd1_;
  ushort *param1 = song_.phrase_.param1_;
  FourCC *cmd2 = song_.phrase_.cmd2_;
  ushort *param2 = song_.phrase_.param2_;

  for (int i = 0; i < PHRASE_COUNT; i++) {
    for (int j = 0; j < 16; j++) {
      if (!song_.phrase_.IsUsed(i)) {
        *data = 0xFF;
        *data2 = 0xFF;
        *cmd1 = I_CMD_NONE;
        *param1 = 0;
        *cmd2 = I_CMD_NONE;
        *param2 = 0;
      }
      data++;
      data2++;
      cmd1++;
      param1++;
      cmd2++;
      param2++;
    };
  }
};

void Project::PurgeInstruments(bool removeFromDisk) {

  bool used[MAX_INSTRUMENT_COUNT];
  for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {
    used[i] = false;
  }

  unsigned char *data = song_.phrase_.instr_;

  for (int i = 0; i < PHRASE_COUNT; i++) {
    for (int j = 0; j < 16; j++) {
      if (*data != 0xFF) {
        NAssert(*data < MAX_INSTRUMENT_COUNT);
        used[*data] = true;
      }
      data++;
    }
  }

  InstrumentBank *bank = GetInstrumentBank();
  for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {
    if (!used[i]) {
      I_Instrument *instrument = bank->GetInstrument(i);
      instrument->Purge();
    }
  }

  // now see if any samples isn't used and get rid if them if needed

  if (removeFromDisk) {

    // clear used flag

    bool iUsed[MAX_PIG_SAMPLES];
    for (int i = 0; i < MAX_PIG_SAMPLES; i++) {
      iUsed[i] = false;
    }

    // flag all samples actually used

    for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {
      I_Instrument *instrument = bank->GetInstrument(i);
      if (instrument->GetType() == IT_SAMPLE) {
        SampleInstrument *si = (SampleInstrument *)instrument;
        int index = si->GetSampleIndex();
        if (index >= 0)
          iUsed[index] = true;
      };
    }

    // Now effectively purge all unused sample from disk
    int purged = 0;
    SamplePool *sp = SamplePool::GetInstance();
    for (int i = 0; i < MAX_PIG_SAMPLES; i++) {
      if ((!iUsed[i]) && (sp->GetSource(i - purged))) {
        char projName[MAX_PROJECT_NAME_LENGTH];
        sp->PurgeSample(i - purged, projectName_.GetString().c_str());
        purged++;
      };
    };
  }
};

void Project::RestoreContent(PersistencyDocument *doc) {
  bool attr = doc->NextAttribute();
  doc->version_ = 32;
  int tableRatio = 0;
  while (attr) {
    if (!strcmp(doc->attrname_, "VERSION")) {
      doc->version_ = int(atof(doc->attrval_) * 100);
    }
    if (!strcmp(doc->attrname_, "TABLERATIO")) {
      tableRatio = atoi(doc->attrval_);
    }
    attr = doc->NextAttribute();
  }
  if (!tableRatio)
    tableRatio = (doc->version_ <= 32) ? 2 : 1;
  SyncMaster::GetInstance()->SetTableRatio(tableRatio);

  // Now loop on all variables
  bool elem = doc->FirstChild();
  while (elem) {
    bool attr = doc->NextAttribute();
    char name[24];
    char value[24];
    while (attr) {
      if (!strcmp(doc->attrname_, "NAME")) {
        strcpy(name, doc->attrval_);
      }
      if (!strcmp(doc->attrname_, "VALUE")) {
        strcpy(value, doc->attrval_);
      }
      attr = doc->NextAttribute();
    }
    Variable *v = FindVariable(name);
    if (v) {
      v->SetString(value);
    }
    elem = doc->NextSibling();
  }
}

void Project::SaveContent(tinyxml2::XMLPrinter *printer) {

  // store project version
  printer->PushAttribute("VERSION", PROJECT_NUMBER);

  // store table ratio if not one
  int tableRatio = SyncMaster::GetInstance()->GetTableRatio();
  if (tableRatio != 1) {
    printer->PushAttribute("TABLERATIO", tableRatio);
  }

  // save all of the project's parameters
  auto it = begin();
  for (size_t i = 0; i < size(); i++) {
    printer->OpenElement("PARAMETER");
    printer->PushAttribute("NAME", (*it)->GetName());
    printer->PushAttribute("VALUE", (*it)->GetString().c_str());
    printer->CloseElement();
    it++;
  }
};

void Project::buildMidiDeviceList() {
  if (midiDeviceList_) {
    for (int i = 0; i < midiDeviceListSize_; i++) {
      SAFE_FREE(midiDeviceList_[i]);
    }
    SAFE_FREE(midiDeviceList_);
  }
  midiDeviceListSize_ = MidiService::GetInstance()->Size();
  midiDeviceList_ = (char **)SYS_MALLOC(midiDeviceListSize_ * sizeof(char *));
  auto midiService = MidiService::GetInstance();
  midiService->Begin();
  for (int i = 0; i < midiDeviceListSize_; i++) {
    std::string deviceName = midiService->CurrentItem().GetName();
    midiDeviceList_[i] = (char *)malloc(sizeof(char *) * deviceName.size() + 1);
    strcpy(midiDeviceList_[i], deviceName.c_str());
    midiService->Next();
  };
};

void Project::OnTempoTap() {

  unsigned long now = System::GetInstance()->GetClock();

  if (tempoTapCount_ != 0) {
    // count last tick tempo and see if in range
    unsigned millisec = now - lastTap_[tempoTapCount_ - 1];
    int t = int(60000 / (float)millisec);
    if (t > 30) {
      if (tempoTapCount_ == MAX_TAP) {
        for (int i = 0; i < int(tempoTapCount_) - 1; i++) {
          lastTap_[i] = lastTap_[i + 1];
        }
      } else {
        tempoTapCount_++;
      }
      int tempo =
          int(60000 * (tempoTapCount_ - 1) / (float)(now - lastTap_[0]));
      Variable *v = FindVariable(VAR_TEMPO);
      v->SetInt(tempo);
    } else {
      tempoTapCount_ = 1;
    }
  } else {
    tempoTapCount_ = 1;
  }
  lastTap_[tempoTapCount_ - 1] = now;
}
