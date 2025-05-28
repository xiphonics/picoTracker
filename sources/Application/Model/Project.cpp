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
    : Persistent("PROJECT"), VariableContainer(&variables_), song_(),
      tempoNudge_(0), tempo_(FourCC::VarTempo, DEFAULT_TEMPO),
      masterVolume_(FourCC::VarMasterVolume, 100),
      channelVolume1_(FourCC::VarChannel1Volume, 60),
      channelVolume2_(FourCC::VarChannel2Volume, 60),
      channelVolume3_(FourCC::VarChannel3Volume, 60),
      channelVolume4_(FourCC::VarChannel4Volume, 60),
      channelVolume5_(FourCC::VarChannel5Volume, 60),
      channelVolume6_(FourCC::VarChannel6Volume, 60),
      channelVolume7_(FourCC::VarChannel7Volume, 60),
      channelVolume8_(FourCC::VarChannel8Volume, 60),
      wrap_(FourCC::VarWrap, false), transpose_(FourCC::VarTranspose, 0),
      scale_(FourCC::VarScale, scaleNames, numScales, 0),
      scaleRoot_(FourCC::VarScaleRoot, noteNames, 12, 0),
      projectName_(FourCC::VarProjectName, name) {

  this->variables_.insert(variables_.end(), &tempo_);
  this->variables_.insert(variables_.end(), &masterVolume_);

  // Add individual channel volume variables to the container
  this->variables_.insert(variables_.end(), &channelVolume1_);
  this->variables_.insert(variables_.end(), &channelVolume2_);
  this->variables_.insert(variables_.end(), &channelVolume3_);
  this->variables_.insert(variables_.end(), &channelVolume4_);
  this->variables_.insert(variables_.end(), &channelVolume5_);
  this->variables_.insert(variables_.end(), &channelVolume6_);
  this->variables_.insert(variables_.end(), &channelVolume7_);
  this->variables_.insert(variables_.end(), &channelVolume8_);

  this->variables_.insert(variables_.end(), &wrap_);
  this->variables_.insert(variables_.end(), &transpose_);
  this->variables_.insert(variables_.end(), &scale_);
  scale_.SetInt(0);
  this->variables_.insert(variables_.end(), &scaleRoot_);
  scaleRoot_.SetInt(0); // Default to C (0)
  this->variables_.insert(variables_.end(), &projectName_);

  // Project name is now managed through the WatchedVariable

  alignas(
      InstrumentBank) static char instrumentBankMemBuf[sizeof(InstrumentBank)];
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
  Variable *v = FindVariable(FourCC::VarScale);
  NAssert(v);
  return v->GetInt();
}

uint8_t Project::GetScaleRoot() {
  Variable *v = FindVariable(FourCC::VarScaleRoot);
  NAssert(v);
  return v->GetInt();
}

int Project::GetTempo() {
  Variable *v = FindVariable(FourCC::VarTempo);
  NAssert(v);
  int tempo = v->GetInt() + tempoNudge_;
  return tempo;
};

int Project::GetMasterVolume() {
  Variable *v = FindVariable(FourCC::VarMasterVolume);
  NAssert(v);
  return v->GetInt();
}

int Project::GetChannelVolume(int channel) {
  // Return the appropriate channel volume variable
  switch (channel) {
  case 0:
    return channelVolume1_.GetInt();
  case 1:
    return channelVolume2_.GetInt();
  case 2:
    return channelVolume3_.GetInt();
  case 3:
    return channelVolume4_.GetInt();
  case 4:
    return channelVolume5_.GetInt();
  case 5:
    return channelVolume6_.GetInt();
  case 6:
    return channelVolume7_.GetInt();
  case 7:
    return channelVolume8_.GetInt();
  default:
    NAssert(false);
    return 0;
  }
};

void Project::GetProjectName(char *name) {
  Variable *v = FindVariable(FourCC::VarProjectName);
  strcpy(name, v->GetString().c_str());
}

void Project::SetProjectName(char *name) {
  Variable *v = FindVariable(FourCC::VarProjectName);
  v->SetString(name, true);
}

void Project::NudgeTempo(int value) {
  if ((GetTempo() + tempoNudge_) > 0) {
    tempoNudge_ += value;
  }
};

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
  Variable *v = FindVariable(FourCC::VarTranspose);
  NAssert(v);
  int result = v->GetInt();
  if (result > 0x80) {
    result -= 128;
  }
  return result;
};

bool Project::Wrap() {
  Variable *v = FindVariable(FourCC::VarWrap);
  NAssert(v);
  return v->GetBool();
};

InstrumentBank *Project::GetInstrumentBank() { return instrumentBank_; };

void Project::Update(Observable &o, I_ObservableData *d) {
  // Nothing to do here for now
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
        *cmd1 = FourCC::InstrumentCommandNone;
        *param1 = 0;
        *cmd2 = FourCC::InstrumentCommandNone;
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

  bool used[MAX_INSTRUMENT_COUNT] = {false};
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
      Trace::Debug("Purged Unused instrument [%d]", i);
    }
  }

  // now see if any samples isn't used and get rid if them if needed
  if (removeFromDisk) {
    // clear used flag
    bool isUsed[MAX_SAMPLES] = {false};

    // flag all samples actually used
    for (int i = 0; i < MAX_INSTRUMENT_COUNT; i++) {
      I_Instrument *instrument = bank->GetInstrument(i);
      if (instrument->GetType() == IT_SAMPLE) {
        SampleInstrument *si = (SampleInstrument *)instrument;
        int index = si->GetSampleIndex();
        if (index >= 0)
          isUsed[index] = true;
      };
    }

    // Now remove all unused samples from disk
    int purged = 0;
    SamplePool *sp = SamplePool::GetInstance();
    for (int i = 0; i < MAX_SAMPLES; i++) {
      if ((!isUsed[i]) && (sp->GetSource(i - purged))) {
        char projName[MAX_PROJECT_NAME_LENGTH];
        sp->PurgeSample(i - purged, projectName_.GetString().c_str());
        Trace::Debug("Purged sample [%d]", i - purged);
        purged++;
      } else {
        Trace::Debug("Sample [%d] not purged", i);
      }
    };
    Trace::Debug("Purged %d samples", purged);
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
  auto it = variables_.begin();
  for (size_t i = 0; i < variables_.size(); i++) {
    printer->OpenElement("PARAMETER");
    printer->PushAttribute("NAME", (*it)->GetName());
    printer->PushAttribute("VALUE", (*it)->GetString().c_str());
    printer->CloseElement();
    it++;
  }
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
      Variable *v = FindVariable(FourCC::VarTempo);
      // ensure tempo is within range
      tempo = std::clamp((unsigned short)tempo, MIN_TEMPO, MAX_TEMPO);
      v->SetInt(tempo);
    } else {
      tempoTapCount_ = 1;
    }
  } else {
    tempoTapCount_ = 1;
  }
  lastTap_[tempoTapCount_ - 1] = now;
}
