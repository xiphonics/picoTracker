#include "InstrumentView.h"
#include "Application/Instruments/MacroInstrument.h"
#include "Application/Instruments/MidiInstrument.h"
#include "Application/Instruments/SIDInstrument.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Config.h"
#include "Application/Views/ImportView.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIIntVarOffField.h"
#include "BaseClasses/UINoteVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "Externals/braids/macro_oscillator.h"
#include "ModalDialogs/MessageBox.h"
#include "ModalDialogs/TextInputModalView.h"
#include "System/System/System.h"
#include <Application/Utils/stringutils.h>
#include <cstdint>
#include <nanoprintf.h>

static void ChangeInstrumentTypeCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    ((InstrumentView &)v).onInstrumentTypeChange();
    Trace::Log("INSTRUMENTVIEW", "instrument type changed!!");

    // clear instrument modified flag
    ((InstrumentView &)v).clearInstrumentModified();
  }
};

InstrumentView::InstrumentView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), instrumentType_(FourCC::VarInstrumentType,
                                          InstrumentTypeNames, IT_LAST, 0) {

  project_ = data->project_;

  GUIPoint position = GUIPoint(5, 1);
  typeIntVarField_.emplace_back(position, *&instrumentType_, "Type: %s", 0,
                                IT_LAST - 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*typeIntVarField_.rbegin()));
  (*typeIntVarField_.rbegin()).AddObserver(*this);
  lastFocusID_ = FourCC::VarInstrumentType;

  // Create the name field with the actual instrument variable
  I_Instrument *instr = getInstrument();
  if (instr) {
    // NONE dont have a name field
    if (instr->GetType() != IT_NONE) {
      position._y = 3;
      addNameTextField(instr, position);
    }
  }

  // add ui action fields for exporting and importing instrument settings
  position._y = 2;

  actionField_.emplace_back("Import", FourCC::ActionImport, position);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);
  lastFocusID_ = FourCC::ActionImport;

  position._x += 8;
  actionField_.emplace_back("Export", FourCC::ActionExport, position);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);
  lastFocusID_ = FourCC::ActionExport;
}

InstrumentView::~InstrumentView() {}

void InstrumentView::addNameTextField(I_Instrument *instr, GUIPoint &position) {
  // Create a NameVariable that bridges between the UITextField and the
  // instrument's name
  nameVariables_.emplace_back(instr);
  Variable &nameVar = *nameVariables_.rbegin();

  auto label =
      etl::make_string_with_capacity<MAX_UITEXTFIELD_LABEL_LENGTH>("name: ");

  // Create a default name based on the instrument's display name
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> defaultName = instr->GetDisplayName();

  nameTextField_.emplace_back(nameVar, position, label, FourCC::InstrumentName,
                              defaultName);
  fieldList_.insert(fieldList_.end(), &(*nameTextField_.rbegin()));
}

I_Instrument *InstrumentView::getInstrument() {
  int id = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  return bank->GetInstrument(id);
};

void InstrumentView::onInstrumentTypeChange() {
  auto nuType = (InstrumentType)instrumentType_.GetInt();
  Trace::Log("INSTRUMENTVIEW", "UPDATE type:%d", nuType);
  I_Instrument *old = getInstrument();

  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();

  auto id = viewData_->currentInstrumentID_;
  // release prev instrument back to available pool
  if (old != nullptr) {
    // first check if the instrument type actually changed, because could be
    // user is at end of instrument list and just keeps pressing key combo to
    // trigger next instrument event again and again
    if (old->GetType() == nuType) {
      return;
    }
    bank->releaseInstrument(viewData_->currentInstrumentID_);
  }

  // now assign new instrument type to the current instrument slot id
  bank->GetNextAndAssignID(nuType, id);

  currentType_ = nuType;

  refreshInstrumentFields(old);
}

void InstrumentView::onInstrumentChange() {

  ClearFocus();

  I_Instrument *old = getInstrument();
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();

  if (getInstrument() != old) {
    getInstrument()->RemoveObserver(*this);
  };

  // update type field to match current instrument
  ((WatchedVariable *)&instrumentType_)->SetInt(getInstrument()->GetType());

  refreshInstrumentFields(old);
};

void InstrumentView::refreshInstrumentFields(const I_Instrument *old) {
  for (auto &f : intVarField_) {
    f.RemoveObserver(*this);
  }
  for (auto &f : bigHexVarField_) {
    f.RemoveObserver(*this);
  }
  for (auto &f : intVarOffField_) {
    f.RemoveObserver(*this);
  }
  for (auto &f : bitmaskVarField_) {
    f.RemoveObserver(*this);
  }

  fieldList_.clear();
  intVarField_.clear();
  noteVarField_.clear();
  staticField_.clear();
  bigHexVarField_.clear();
  intVarOffField_.clear();
  bitmaskVarField_.clear();
  nameTextField_.clear();
  nameVariables_.clear();

  // first put back the type field as its shown on *all* instrument types
  fieldList_.insert(fieldList_.end(), &(*typeIntVarField_.rbegin()));
  lastFocusID_ = FourCC::VarInstrumentType;

  // Re-add the action fields for export and import only if not IT_NONE
  if (instrumentType_.GetInt() != IT_NONE) {
    for (auto &action : actionField_) {
      fieldList_.insert(fieldList_.end(), &action);
      action.AddObserver(*this); // Make sure observers are re-added
    }
  } else {
    // add back only the import field for IT_NONE
    // bit of a hack !!since we just assume that import is the first action
    // field
    fieldList_.insert(fieldList_.end(), &(*actionField_.begin()));
    (*actionField_.rbegin()).AddObserver(*this);
  }

  // Create a new nameTextField_ if the instrument type supports it
  if (instrumentType_.GetInt() != IT_NONE) {
    I_Instrument *instr = getInstrument();
    if (instr) {
      GUIPoint position = GetAnchor();
      addNameTextField(instr, position);
    }
  }

  InstrumentType it = getInstrument()->GetType();
  switch (it) {
  case IT_NONE:
    fillNoneParameters();
    break;
  case IT_MIDI:
    fillMidiParameters();
    break;
  case IT_SID:
    fillSIDParameters();
    break;
  case IT_SAMPLE:
    fillSampleParameters();
    break;
  case IT_OPAL:
    fillOpalParameters();
    break;
  case IT_LAST:
    // NA
    break;
  };

  for (auto field : fieldList_) {
    if (((UIIntVarField *)field)->GetVariableID() == lastFocusID_) {
      SetFocus(field);
      break;
    }
  }

  // observer all var fields so we can mark the instrument as modified
  // to be able to show confirmation dialog when switching instrument type
  for (auto &f : intVarField_) {
    f.AddObserver(*this);
  }
  for (auto &f : bigHexVarField_) {
    f.AddObserver(*this);
  }
  for (auto &f : intVarOffField_) {
    f.AddObserver(*this);
  }
  for (auto &f : bitmaskVarField_) {
    f.AddObserver(*this);
  }

  if (getInstrument() != old) {
    getInstrument()->AddObserver(*this);
  }
}

void InstrumentView::fillNoneParameters() {}

void InstrumentView::fillMacroParameters() {
  int i = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  MacroInstrument *instrument = (MacroInstrument *)instr;
  GUIPoint position = GetAnchor();

  // offset y to account for instrument type and export/import fields
  position._y += 1;

  position._y += 1;
  Variable *v = instrument->FindVariable(FourCC::MacroInstrumentShape);
  intVarField_.emplace_back(position, *v, "shape: %s", 0,
                            braids::MACRO_OSC_SHAPE_LAST - 2, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::MacroInstrmentTimbre);
  intVarField_.emplace_back(position, *v, "timbre: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::MacroInstrumentColor);
  intVarField_.emplace_back(position, *v, "color: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::MacroInstrumentAttack);
  intVarField_.emplace_back(position, *v, "attack: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::MacroInstrumentDecay);
  intVarField_.emplace_back(position, *v, "decay: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::MacroInstrumentSignature);
  intVarField_.emplace_back(position, *v, "signature: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
}

void InstrumentView::fillSampleParameters() {

  int i = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  SampleInstrument *instrument = (SampleInstrument *)instr;

  GUIPoint position = GetAnchor();

  // offset y to account for instrument type and export/import fields
  position._y += 1;

  Variable *v = instrument->FindVariable(FourCC::SampleInstrumentSample);
  SamplePool *sp = SamplePool::GetInstance();
  intVarField_.emplace_back(position, *v, "sample: %.19s", 0,
                            sp->GetNameListSize() - 1, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentVolume);
  intVarField_.emplace_back(position, *v, "volume: %d [%2.2X]", 0, 255, 1, 10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentPan);
  intVarField_.emplace_back(position, *v, "pan: %2.2X", 0, 0xFE, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentRootNote);
  noteVarField_.emplace_back(position, *v, "root note: %s", 0, 0x7F, 1, 0x0C);
  fieldList_.insert(fieldList_.end(), &(*noteVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentFineTune);
  intVarField_.emplace_back(position, *v, "detune: %2.2X", 0, 255, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentCrushVolume);
  intVarField_.emplace_back(position, *v, "drive: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentCrush);
  intVarField_.emplace_back(position, *v, "crush: %d", 1, 0x10, 1, 4);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentDownsample);
  intVarField_.emplace_back(position, *v, "downsample: %d", 0, 8, 1, 4);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "flt cut/res:");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._x += 13;
  v = instrument->FindVariable(FourCC::SampleInstrumentFilterCutOff);
  intVarField_.emplace_back(position, *v, "%2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._x += 3;
  v = instrument->FindVariable(FourCC::SampleInstrumentFilterResonance);
  intVarField_.emplace_back(position, *v, "%2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._x -= 16;

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentFilterType);
  intVarField_.emplace_back(position, *v, "type: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentFilterMode);
  intVarField_.emplace_back(position, *v, "Mode: %s", 0, 2, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentInterpolation);
  intVarField_.emplace_back(position, *v, "interpolation: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentLoopMode);
  intVarField_.emplace_back(position, *v, "loop mode: %s", 0, SILM_LAST - 1, 1,
                            1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentStart);
  bigHexVarField_.emplace_back(position, *v, 7, "start: %7.7X", 0,
                               instrument->GetSampleSize() - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentLoopStart);
  bigHexVarField_.emplace_back(position, *v, 7, "loop start: %7.7X", 0,
                               instrument->GetSampleSize() - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentEnd);
  bigHexVarField_.emplace_back(position, *v, 7, "loop end: %7.7X", 0,
                               instrument->GetSampleSize() - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));

  v = instrument->FindVariable(FourCC::SampleInstrumentTableAutomation);
  position._y += 2;
  intVarField_.emplace_back(position, *v, "automation: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SampleInstrumentTable);
  intVarOffField_.emplace_back(position, *v, "table: %2.2X", 0x00,
                               TABLE_COUNT - 1, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));
};

void InstrumentView::fillSIDParameters() {

  int i = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  SIDInstrument *instrument = (SIDInstrument *)bank->GetInstrument(i);
  GUIPoint position = GetAnchor();

  // offset y to account for instrument type, name and export/import fields
  position._y += 2;

  staticField_.emplace_back(position, instrument->GetChipName().c_str());
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y += 1;

  Variable *v = instrument->FindVariable(FourCC::SIDInstrumentOSCNumber);
  intVarField_.emplace_back(position, *v, "OSC: %1.1X", 0, 0x2, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  v = instrument->FindVariable(FourCC::SIDInstrumentPulseWidth);
  intVarField_.emplace_back(position, *v, "VPW: %2.2X", 0, 0xFFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  switch (instrument->GetChip()) {
  case SID1:
    v = instrument->FindVariable(FourCC::SIDInstrument1Waveform);
    break;
  case SID2:
    v = instrument->FindVariable(FourCC::SIDInstrument2Waveform);
    break;
  }

  // Only support independent waveforms for the moment
  intVarField_.emplace_back(position, *v, "WF: %s", 0, DWF_LAST - 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SIDInstrumentVSync);
  intVarField_.emplace_back(position, *v, "Sync: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SIDInstrumentRingModulator);
  intVarField_.emplace_back(position, *v, "RING: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SIDInstrumentADSR);
  bigHexVarField_.emplace_back(
      UIBigHexVarField(position, *v, 4, "A/D/S/R: %4.4X", 0, 0xFFFF, 16, true));
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "== CHIP SETTINGS ==");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::SIDInstrumentFilterOn);
  intVarField_.emplace_back(position, *v, "Filter: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  switch (instrument->GetChip()) {
  case SID1:
    v = instrument->FindVariable(FourCC::SIDInstrument1FilterCut);
    break;
  case SID2:
    v = instrument->FindVariable(FourCC::SIDInstrument2FilterCut);
    break;
  }
  intVarField_.emplace_back(position, *v, "Flt cut: %1.1X", 0, 0x7FF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  switch (instrument->GetChip()) {
  case SID1:
    v = instrument->FindVariable(FourCC::SIDInstrument1FilterResonance);
    break;
  case SID2:
    v = instrument->FindVariable(FourCC::SIDInstrument2FilterResonance);
    break;
  }
  intVarField_.emplace_back(position, *v, "Flt Res: %1.1X", 0, 0xF, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  switch (instrument->GetChip()) {
  case SID1:
    v = instrument->FindVariable(FourCC::SIDInstrument1FilterMode);
    break;
  case SID2:
    v = instrument->FindVariable(FourCC::SIDInstrument2FilterMode);
    break;
  }
  intVarField_.emplace_back(position, *v, "Flt mode: %s", 0, DFM_LAST - 1, 1,
                            1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  switch (instrument->GetChip()) {
  case SID1:
    v = instrument->FindVariable(FourCC::SIDInstrument1Volume);
    break;
  case SID2:
    v = instrument->FindVariable(FourCC::SIDInstrument2Volume);
    break;
  }
  intVarField_.emplace_back(position, *v, "Volume: %1.1X", 0, 0xF, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
};

void InstrumentView::fillMidiParameters() {

  int i = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  MidiInstrument *instrument = (MidiInstrument *)instr;
  GUIPoint position = GetAnchor();

  // offset y to account for instrument type, name and export/import fields
  position._y += 3;

  Variable *v = instrument->FindVariable(FourCC::MidiInstrumentChannel);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "channel: %2.2d", 0, 0x0F, 1, 0x04, 1));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::MidiInstrumentVolume);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "volume: %2.2X", 0, 0xFF, 1, 0x10));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::MidiInstrumentNoteLength);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "length: %2.2X", 0, 0xFF, 1, 0x10));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  v = instrument->FindVariable(FourCC::MidiInstrumentTableAutomation);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "automation: %s", 0, 1, 1, 1));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::MidiInstrumentTable);
  intVarOffField_.emplace_back(
      UIIntVarOffField(position, *v, "table: %2.2X", 0, 0x7F, 1, 0x10));
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));
};

void InstrumentView::fillOpalParameters() {
  int i = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  OpalInstrument *instrument = (OpalInstrument *)instr;
  GUIPoint position = GetAnchor();

  uint8_t savex = 0;

  // extra y spacing to allow for gap between export/import and parameters
  position._y += 2;
  Variable *v = instrument->FindVariable(FourCC::OPALInstrumentAlgorithm);
  intVarField_.emplace_back(position, *v, "algorithm:     %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::OPALInstrumentDeepTremeloVibrato);
  bitmaskVarField_.emplace_back(
      UIBitmaskVarField(position, *v, "deep tr/vb:    %02b", 2));
  fieldList_.insert(fieldList_.end(), &(*bitmaskVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::OPALInstrumentFeedback);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "feedback:      %1.1X", 0, 0x07, 1, 1, 0));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "               Op 1 Op 2");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp1Level);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "level:         %2.2X", 0, 63, 1, 1, 0));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  savex = position._x;
  position._x += 20;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp2Level);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "%2.2X", 0, 63, 1, 1, 0));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  position._x = savex;

  position._y += 1;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp1Multiplier);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "multipler:     %1.1X", 0, 15, 1, 1, 0));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  savex = position._x;
  position._x += 20;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp2Multiplier);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "%1.1X", 0, 15, 1, 1, 0));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  position._x = savex;

  position._y += 1;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp1ADSR);
  bigHexVarField_.emplace_back(UIBigHexVarField(
      position, *v, 4, "A/D/S/R:       %4.4X", 0, 0xFFFF, 16, true));
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));

  savex = position._x;
  position._x += 20;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp2ADSR);
  bigHexVarField_.emplace_back(
      UIBigHexVarField(position, *v, 4, "%4.4X", 0, 0xFFFF, 16, true));
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  position._x = savex;

  position._y += 1;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp1WaveShape);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "shape:         %s", 0, 7, 1, 1));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._x += 20;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp2WaveShape);
  intVarField_.emplace_back(UIIntVarField(position, *v, "%s", 0, 7, 1, 1));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  position._x = savex;

  position._y += 1;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp1TremVibSusKSR);
  bitmaskVarField_.emplace_back(
      UIBitmaskVarField(position, *v, "TR/VB/SU/KSR:  %04b", 4));
  fieldList_.insert(fieldList_.end(), &(*bitmaskVarField_.rbegin()));

  position._x += 20;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp2TremVibSusKSR);
  bitmaskVarField_.emplace_back(UIBitmaskVarField(position, *v, "%04b", 4));
  fieldList_.insert(fieldList_.end(), &(*bitmaskVarField_.rbegin()));
  position._x = savex;

  position._y += 1;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp1KeyScaleLevel);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "keyscale:      %s", 0, 3, 1, 1));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._x += 20;
  v = instrument->FindVariable(FourCC::OPALInstrumentOp2KeyScaleLevel);
  intVarField_.emplace_back(UIIntVarField(position, *v, "%s", 0, 3, 1, 1));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  position._x = savex;
};

void InstrumentView::warpToNext(int offset) {
  int instrument = viewData_->currentInstrumentID_ + offset;
  if (instrument >= MAX_INSTRUMENT_COUNT) {
    instrument = instrument - MAX_INSTRUMENT_COUNT;
  };
  if (instrument < 0) {
    instrument = MAX_INSTRUMENT_COUNT + instrument;
  };
  viewData_->currentInstrumentID_ = instrument;
  onInstrumentChange();
  isDirty_ = true;
};

void InstrumentView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed)
    return;

  isDirty_ = false;

  Player *player = Player::GetInstance();

  if (viewMode_ == VM_NEW) {
    if (mask == EPBM_ENTER) {
      UIIntVarField *field = (UIIntVarField *)GetFocus();
      Variable &v = field->GetVariable();
      switch (v.GetID()) {
      case FourCC::SampleInstrumentSample: {
        if (!player->IsRunning()) {
          // First check if the samplelib exists
          bool samplelibExists =
              FileSystem::GetInstance()->exists(SAMPLES_LIB_DIR);

          if (!samplelibExists) {
            MessageBox *mb =
                new MessageBox(*this, "Can't access the samplelib", MBBF_OK);
            DoModal(mb);
          } else {
            // Go to import sample
            ViewType vt = VT_IMPORT;
            ViewEvent ve(VET_SWITCH_VIEW, &vt);
            SetChanged();
            NotifyObservers(&ve);
          }
        } else {
          MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
          DoModal(mb);
        }
        break;
      }
      case FourCC::SampleInstrumentTable: {
        int next = TableHolder::GetInstance()->GetNext();
        if (next != NO_MORE_TABLE) {
          v.SetInt(next);
          isDirty_ = true;
        }
        break;
      }
      default:
        break;
      }
      mask &= (0xFFFF - EPBM_ENTER);
    }
  }

  if (viewMode_ == VM_CLONE) {
    if ((mask & EPBM_ENTER) && (mask & EPBM_ALT)) {
      UIIntVarField *field = (UIIntVarField *)GetFocus();
      mask &= (0xFFFF - EPBM_ENTER);
      Variable &v = field->GetVariable();
      int current = v.GetInt();
      if (current == -1)
        return;

      int next = TableHolder::GetInstance()->Clone(current);
      if (next != NO_MORE_TABLE) {
        v.SetInt(next);
        isDirty_ = true;
      }
    }
    mask &= (0xFFFF - (EPBM_ENTER | EPBM_ALT));
  };

  if (viewMode_ == VM_SELECTION) {
  } else {
    viewMode_ = VM_NORMAL;
  }

  FieldView::ProcessButtonMask(mask, pressed);

  // EDIT Modifier
  if (mask & EPBM_EDIT) {
    if (mask & EPBM_LEFT)
      warpToNext(-1);
    if (mask & EPBM_RIGHT)
      warpToNext(+1);
    if (mask & EPBM_DOWN)
      warpToNext(-16);
    if (mask & EPBM_UP)
      warpToNext(+16);
    if (mask & EPBM_ENTER) { // Allow cut instrument
      if (getInstrument()->GetType() == IT_SAMPLE) {
        if (GetFocus() == *fieldList_.begin()) {
          int i = viewData_->currentInstrumentID_;
          InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
          I_Instrument *instr = bank->GetInstrument(i);
          instr->Purge();
          //                   Variable *v=instr->FindVariable(SIP_SAMPLE) ;
          //                   v->SetInt(-1) ;
          isDirty_ = true;
        }
      }

      // Check if on table
      UIIntVarField *field = (UIIntVarField *)GetFocus();
      if ((field->GetVariableID() == FourCC::SampleInstrumentTable) ||
          (field->GetVariableID() == FourCC::MidiInstrumentTable)) {
        Variable &v = field->GetVariable();
        v.SetInt(-1);
        isDirty_ = true;
      };
    }
    if (mask & EPBM_ALT) {
      viewMode_ = VM_CLONE;
    };
  } else {

    // ENTER modifier
    if (mask == EPBM_ENTER) {
      FourCC varID = ((UIIntVarField *)GetFocus())->GetVariableID();
      if ((varID == FourCC::SampleInstrumentTable) ||
          (varID == FourCC::MidiInstrumentTable) ||
          (varID == FourCC::SampleInstrumentSample)) {
        viewMode_ = VM_NEW;
      };
    } else {

      // NAV Modifier
      if (mask & EPBM_NAV) {
        if (mask & EPBM_LEFT) {
          ViewType vt = VT_PHRASE;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);

          // remove listening when leaving this screen
          getInstrument()->RemoveObserver(*this);
          ((WatchedVariable *)&instrumentType_)->RemoveObserver(*this);

          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_DOWN) {

          // Go to table view

          ViewType vt = VT_TABLE2;

          int i = viewData_->currentInstrumentID_;
          InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
          I_Instrument *instr = bank->GetInstrument(i);
          int table = instr->GetTable();
          if (table != VAR_OFF) {
            viewData_->currentTable_ = table;
          }
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_PLAY) {
          player->OnStartButton(PM_PHRASE, viewData_->songX_, true,
                                viewData_->chainRow_);
        }
      } else {
        // No modifier
        if (mask & EPBM_PLAY) {
          player->OnStartButton(PM_PHRASE, viewData_->songX_, false,
                                viewData_->chainRow_);
        }
      }
    }
  }

  UIIntVarField *field = (UIIntVarField *)GetFocus();
  if (field) {
    lastFocusID_ = field->GetVariableID();
  }
};

void InstrumentView::DrawView() {

  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title

  char title[20];
  SetColor(CD_NORMAL);
  npf_snprintf(title, sizeof(title), "Instrument %2.2X",
               viewData_->currentInstrumentID_);
  DrawString(pos._x, pos._y, title, props);

  drawBattery(props);

  // Draw fields

  FieldView::Redraw();
  drawMap();
};

void InstrumentView::OnFocus() {
  Trace::Log("INSTRUMENTVIEW", "onFocus");

  // Make sure we're observing the instrument type
  ((WatchedVariable *)&instrumentType_)->AddObserver(*this);

  // Get the current instrument
  I_Instrument *instr = getInstrument();
  if (instr) {
    // Update the instrument type field to match the current instrument
    InstrumentType currentType = instr->GetType();
    Trace::Log("INSTRUMENTVIEW", "Current instrument type: %d", currentType);

    // Only update if the type has changed
    if (instrumentType_.GetInt() != currentType) {
      Trace::Log("INSTRUMENTVIEW", "Updating instrument type from %d to %d",
                 instrumentType_.GetInt(), currentType);
      ((WatchedVariable *)&instrumentType_)->SetInt(currentType);
    }
  }

  // Force a full refresh of the view
  onInstrumentChange();
}

void InstrumentView::Update(Observable &o, I_ObservableData *data) {

  if (!hasFocus_) {
    return;
  }

  uintptr_t fourcc = (uintptr_t)data;

  switch (fourcc) {
  case FourCC::VarInstrumentType: {
    Trace::Debug("INSTRUMENTVIEW", "instrument type change:%d",
                 (InstrumentType)instrumentType_.GetInt());
    // confirm user wants to change instrument type &lose changes
    Player *player = Player::GetInstance();
    if (!player->IsRunning()) {
      if (instrumentModified_) {
        MessageBox *mb = new MessageBox(*this, "Change Instrument &",
                                        "lose settings?", MBBF_YES | MBBF_NO);
        DoModal(mb, ChangeInstrumentTypeCallback);
      } else {
        onInstrumentTypeChange();
      }
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
    break;
  }
  case FourCC::ActionExport: {
    handleInstrumentExport();
  } break;
  case FourCC::ActionImport: {
    // Switch to the InstrumentImportView
    ViewType vt = VT_INSTRUMENT_IMPORT;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
  } break;
  default:
    if (fourcc != 0) {
      instrumentModified_ = true;
    }
    break;
  }
}

void InstrumentView::handleInstrumentExport() {
  // Get current instrument using its id
  I_Instrument *instrument =
      viewData_->project_->GetInstrumentBank()->GetInstrument(
          viewData_->currentInstrumentID_);

  // Check if the instrument has a name set
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> name = instrument->GetDisplayName();
  // Check if the name is empty, the default value, or matches the default
  // instrument type name
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> defaultTypeName =
      instrument->GetDefaultName();

  if (name.empty() || name == defaultTypeName) {
    // Show error message if no name is set
    MessageBox *mb =
        new MessageBox(*this, "Please set a name", "before exporting", MBBF_OK);
    DoModal(mb);
  } else {
    // Export the instrument using the name field
    PersistencyResult result =
        PersistencyService::GetInstance()->ExportInstrument(instrument, name);

    if (result == PERSIST_EXISTS) {
      // File already exists, ask user if they want to override it
      etl::string<strlen("Overwrite existing file: ")> confirmMsg =
          "Overwrite existing file?";
      MessageBox *mb = new MessageBox(*this, confirmMsg.c_str(), name.c_str(),
                                      MBBF_YES | MBBF_NO);

      // Store the instrument and name for use in the callback
      exportInstrument_ = instrument;
      exportName_ = name;

      DoModal(mb, [](View &v, ModalView &dialog) {
        if (dialog.GetReturnCode() == MBL_YES) {
          // User confirmed override, call ExportInstrument with
          // overwrite=true
          InstrumentView &iv = (InstrumentView &)v;

          // Re-export the instrument with overwrite flag set to true
          PersistencyResult result =
              PersistencyService::GetInstance()->ExportInstrument(
                  iv.exportInstrument_, iv.exportName_, true);

          // TODO: unfortunately we can't show the result message here
          // because we're in a modal already and showing a model from result
          // of a modal is not supported
        }
      });
    } else {
      // Create a message with the instrument name
      etl::string<MAX_INSTRUMENT_NAME_LENGTH + strlen("Exported: ")>
          successMsg = "Exported: ";
      successMsg += name;

      const char *message = result == PERSIST_SAVED
                                ? successMsg.c_str()
                                : "Failed to export instrument";
      // Show export result message
      MessageBox *mb = new MessageBox(*this, message, MBBF_OK);
      DoModal(mb);
    }
  }
}

void InstrumentView::AnimationUpdate() {
  // redraw batt gauge on every clock tick (~1Hz) even when not playing
  // and not redrawing due to user cursor navigation
  GUITextProperties props;
  drawBattery(props);
  w_.Flush();
};
