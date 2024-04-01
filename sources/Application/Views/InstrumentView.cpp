#include "InstrumentView.h"
#include "Application/Instruments/MidiInstrument.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Config.h"
#include "ModalDialogs/ImportSampleDialog.h"
#include "ModalDialogs/MessageBox.h"
#include "ModalDialogs/PagedImportSampleDialog.h"
#include "System/System/System.h"

InstrumentView::InstrumentView(GUIWindow &w, ViewData *data)
    : FieldView(w, data) {

  project_ = data->project_;
  lastFocusID_ = 0;
  current_ = 0;
  onInstrumentChange();
}

InstrumentView::~InstrumentView() {}

InstrumentType InstrumentView::getInstrumentType() {
  int i = viewData_->currentInstrument_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instrument = bank->GetInstrument(i);
  return instrument->GetType();
};

void InstrumentView::onInstrumentChange() {

  ClearFocus();

  I_Instrument *old = current_;

  int i = viewData_->currentInstrument_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  current_ = bank->GetInstrument(i);

  if (current_ != old) {
    current_->RemoveObserver(*this);
  };

  fieldList_.clear();
  intVarField_.clear();
  noteVarField_.clear();
  staticField_.clear();
  bigHexVarField_.clear();
  intVarOffField_.clear();

  InstrumentType it = getInstrumentType();
  switch (it) {
  case IT_MIDI:
    fillMidiParameters();
    break;
  case IT_SAMPLE:
    fillSampleParameters();
    break;
  case IT_TINYSYNTH:
    fillTinysynthParameters();
    break;
  };

  SetFocus(*fieldList_.begin());
  auto it2 = fieldList_.begin();
  for (size_t i = 0; i < fieldList_.size(); i++) {
    UIIntVarField &field = (UIIntVarField &)(**it2);
    if (field.GetVariableID() == lastFocusID_) {
      SetFocus(&field);
      break;
    }
    it2++;
  };
  if (current_ != old) {
    current_->AddObserver(*this);
  }
};

void InstrumentView::fillTinysynthParameters() {
  printf("fill Tinysynth params\n");
  int i = viewData_->currentInstrument_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  TinysynthInstrument *instrument = (TinysynthInstrument *)instr;
  GUIPoint position = GetAnchor();
  printf("TINY INSTRUMENT:%d,%d", instrument, position);

  position._y -= 1;
  Variable *v = instrument->FindVariable(TXIP_VOLUME);
  intVarField_.emplace_back(position, *v, "volume: %d [%2.2X]", 0, 255, 1, 10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;

  staticField_.emplace_back(position, "H ADSR  VT");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y += 1;

  staticField_.emplace_back(position, "1");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._x += 2;

  v = instrument->FindVariable(TXIP_H1);
  tinySynthVarField_.emplace_back(position, *v, 4, "%4.4X", 0, 0xFFFF - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*tinySynthVarField_.rbegin()));

  position._x += 6;
  v = instrument->FindVariable(TXIP_V1);
  tinySynthVarField_.emplace_back(position, *v, 2, "%2.2X", 0, 0xFF, 16);
  fieldList_.insert(fieldList_.end(), &(*tinySynthVarField_.rbegin()));

  // == 1

  position._x -= 8;
  position._y += 1;

  staticField_.emplace_back(position, "2");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._x += 2;

  v = instrument->FindVariable(TXIP_H2);
  tinySynthVarField_.emplace_back(position, *v, 4, "%4.4X", 0, 0xFFFF - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*tinySynthVarField_.rbegin()));

  position._x += 6;
  v = instrument->FindVariable(TXIP_V2);
  tinySynthVarField_.emplace_back(position, *v, 2, "%2.2X", 0, 0xFF, 16);
  fieldList_.insert(fieldList_.end(), &(*tinySynthVarField_.rbegin()));

  // == 2
}

void InstrumentView::fillSampleParameters() {

  int i = viewData_->currentInstrument_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  SampleInstrument *instrument = (SampleInstrument *)instr;
  GUIPoint position = GetAnchor();

  //   position._y+=View::fieldSpaceHeight_;
  position._y -= 1;
  Variable *v = instrument->FindVariable(SIP_SAMPLE);
  SamplePool *sp = SamplePool::GetInstance();
  intVarField_.emplace_back(position, *v, "sample: %.19s", 0,
                            sp->GetNameListSize() - 1, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).SetFocus();

  position._y += 2;
  v = instrument->FindVariable(SIP_VOLUME);
  intVarField_.emplace_back(position, *v, "volume: %d [%2.2X]", 0, 255, 1, 10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_PAN);
  intVarField_.emplace_back(position, *v, "pan: %2.2X", 0, 0xFE, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_ROOTNOTE);
  noteVarField_.emplace_back(position, *v, "root note: %s", 0, 0x7F, 1, 0x0C);
  fieldList_.insert(fieldList_.end(), &(*noteVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_FINETUNE);
  intVarField_.emplace_back(position, *v, "detune: %2.2X", 0, 255, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_CRUSHVOL);
  intVarField_.emplace_back(position, *v, "drive: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_CRUSH);
  intVarField_.emplace_back(position, *v, "crush: %d", 1, 0x10, 1, 4);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_DOWNSMPL);
  intVarField_.emplace_back(position, *v, "downsample: %d", 0, 8, 1, 4);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "flt cut/res:");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._x += 13;
  v = instrument->FindVariable(SIP_FILTCUTOFF);
  intVarField_.emplace_back(position, *v, "%2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._x += 3;
  v = instrument->FindVariable(SIP_FILTRESO);
  intVarField_.emplace_back(position, *v, "%2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._x -= 16;

  position._y += 1;
  v = instrument->FindVariable(SIP_FILTMIX);
  intVarField_.emplace_back(position, *v, "type: %2.2X", 0, 0xFF, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_FILTMODE);
  intVarField_.emplace_back(position, *v, "Mode: %s", 0, 2, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  v = instrument->FindVariable(SIP_INTERPOLATION);
  intVarField_.emplace_back(position, *v, "interpolation: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_LOOPMODE);
  intVarField_.emplace_back(position, *v, "loop mode: %s", 0, SILM_LAST - 1, 1,
                            1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_START);
  bigHexVarField_.emplace_back(position, *v, 7, "start: %7.7X", 0,
                               instrument->GetSampleSize() - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_LOOPSTART);
  bigHexVarField_.emplace_back(position, *v, 7, "loop start: %7.7X", 0,
                               instrument->GetSampleSize() - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_END);
  bigHexVarField_.emplace_back(position, *v, 7, "loop end: %7.7X", 0,
                               instrument->GetSampleSize() - 1, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));

  v = instrument->FindVariable(SIP_TABLEAUTO);
  position._y += 2;
  intVarField_.emplace_back(position, *v, "automation: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(SIP_TABLE);
  intVarOffField_.emplace_back(position, *v, "table: %2.2X", 0x00,
                               TABLE_COUNT - 1, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));
};

void InstrumentView::fillMidiParameters() {

  int i = viewData_->currentInstrument_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  MidiInstrument *instrument = (MidiInstrument *)instr;
  GUIPoint position = GetAnchor();

  Variable *v = instrument->FindVariable(MIP_CHANNEL);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "channel: %2.2d", 0, 0x0F, 1, 0x04, 1));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).SetFocus();

  position._y += 1;
  v = instrument->FindVariable(MIP_VOLUME);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "volume: %2.2X", 0, 0xFF, 1, 0x10));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(MIP_NOTELENGTH);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "length: %2.2X", 0, 0xFF, 1, 0x10));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  v = instrument->FindVariable(MIP_TABLEAUTO);
  intVarField_.emplace_back(
      UIIntVarField(position, *v, "automation: %s", 0, 1, 1, 1));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(MIP_TABLE);
  intVarOffField_.emplace_back(
      UIIntVarOffField(position, *v, "table: %2.2X", 0, 0x7F, 1, 0x10));
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));
};

void InstrumentView::warpToNext(int offset) {
  int instrument = viewData_->currentInstrument_ + offset;
  if (instrument >= MAX_INSTRUMENT_COUNT) {
    instrument = instrument - MAX_INSTRUMENT_COUNT;
  };
  if (instrument < 0) {
    instrument = MAX_INSTRUMENT_COUNT + instrument;
  };
  viewData_->currentInstrument_ = instrument;
  onInstrumentChange();
  isDirty_ = true;
};

void InstrumentView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed)
    return;

  isDirty_ = false;

  Player *player = Player::GetInstance();

  if (viewMode_ == VM_NEW) {
    if (mask == EPBM_A) {
      UIIntVarField *field = (UIIntVarField *)GetFocus();
      Variable &v = field->GetVariable();
      switch (v.GetID()) {
      case SIP_SAMPLE: {
        if (!player->IsRunning()) {
          // First check if the samplelib exists
          Path sampleLib(SamplePool::GetInstance()->GetSampleLib());
          if (FileSystem::GetInstance()->GetFileType(
                  sampleLib.GetPath().c_str()) != FT_DIR) {
            MessageBox *mb =
                new MessageBox(*this, "Can't access the samplelib", MBBF_OK);
            DoModal(mb);
          } else {
            ;
            // Go to import sample
#ifdef PICOBUILD
            PagedImportSampleDialog *isd = new PagedImportSampleDialog(*this);
#else
            ImportSampleDialog *isd = new ImportSampleDialog(*this);
#endif
            DoModal(isd);
          }
        } else {
          MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
          DoModal(mb);
        }
        break;
      }
      case SIP_TABLE: {
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
      mask &= (0xFFFF - EPBM_A);
    }
  }

  if (viewMode_ == VM_CLONE) {
    if ((mask & EPBM_A) && (mask & EPBM_L)) {
      UIIntVarField *field = (UIIntVarField *)GetFocus();
      mask &= (0xFFFF - EPBM_A);
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
    mask &= (0xFFFF - (EPBM_A | EPBM_L));
  };

  if (viewMode_ == VM_SELECTION) {
  } else {
    viewMode_ = VM_NORMAL;
  }

  FieldView::ProcessButtonMask(mask);

  // B Modifier

  if (mask & EPBM_B) {
    if (mask & EPBM_LEFT)
      warpToNext(-1);
    if (mask & EPBM_RIGHT)
      warpToNext(+1);
    if (mask & EPBM_DOWN)
      warpToNext(-16);
    if (mask & EPBM_UP)
      warpToNext(+16);
    if (mask & EPBM_A) { // Allow cut instrument
      if (getInstrumentType() == IT_SAMPLE) {
        if (GetFocus() == *fieldList_.begin()) {
          int i = viewData_->currentInstrument_;
          InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
          I_Instrument *instr = bank->GetInstrument(i);
          instr->Purge();
          //                   Variable *v=instr->FindVariable(SIP_SAMPLE) ;
          //                   v->SetInt(-1) ;
          isDirty_ = true;
        }
      }

      // Check if on table
      if (GetFocus() == *fieldList_.rbegin()) {
        int i = viewData_->currentInstrument_;
        InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
        I_Instrument *instr = bank->GetInstrument(i);
        Variable *v = instr->FindVariable(SIP_TABLE);
        v->SetInt(-1);
        isDirty_ = true;
      };
    }
    if (mask & EPBM_L) {
      viewMode_ = VM_CLONE;
    };
  } else {

    // A modifier

    if (mask == EPBM_A) {
      FourCC varID = ((UIIntVarField *)GetFocus())->GetVariableID();
      if ((varID == SIP_TABLE) || (varID == MIP_TABLE) ||
          (varID == SIP_SAMPLE)) {
        viewMode_ = VM_NEW;
      };
    } else {

      // R Modifier

      if (mask & EPBM_R) {
        if (mask & EPBM_LEFT) {
          ViewType vt = VT_PHRASE;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_DOWN) {

          // Go to table view

          ViewType vt = VT_TABLE2;

          int i = viewData_->currentInstrument_;
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

        // if (mask&EPBM_RIGHT) {

        //	// Go to import sample

        //		ViewType vt=VT_IMPORT ;
        //		ViewEvent ve(VET_SWITCH_VIEW,&vt) ;
        //		SetChanged();
        //		NotifyObservers(&ve) ;
        //}

        if (mask & EPBM_START) {
          player->OnStartButton(PM_PHRASE, viewData_->songX_, true,
                                viewData_->chainRow_);
        }
      } else {
        // No modifier
        if (mask & EPBM_START) {
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
  sprintf(title, "Instrument %2.2X", viewData_->currentInstrument_);
  DrawString(pos._x, pos._y, title, props);

  // Draw fields

  FieldView::Redraw();
  drawMap();
};

void InstrumentView::OnFocus() { onInstrumentChange(); }

void InstrumentView::Update(Observable &o, I_ObservableData *d) {
  onInstrumentChange();
}
