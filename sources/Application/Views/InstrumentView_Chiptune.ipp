/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#define inner(x) x.format, x.min, x.max, x.step, x.bigStep
#define expand(x) inner(chiptune_instrument_ui_t.x)

void InstrumentView::fillChiptuneParameters() {
  int i = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  ChiptuneInstrument *instrument = (ChiptuneInstrument *)instr;
  GUIPoint position = GetAnchor();

  // extra y spacing to allow for gap between export/import and parameters
  position._y += 2;
  Variable *v = instrument->FindVariable(FourCC::ChiptuneInstrumentWaveform);
  intVarField_.emplace_back(position, *v, expand(wave));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentTranspose);
  intVarField_.emplace_back(position, *v, expand(transpose));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentLevel);
  intVarField_.emplace_back(position, *v, expand(level));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentBurst);
  intVarOffField_.emplace_back(position, *v, expand(burst));
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentArpSpeed);
  intVarField_.emplace_back(position, *v, expand(arp));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentLength);
  intVarOffField_.emplace_back(position, *v, expand(length));
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentTable);
  intVarOffField_.emplace_back(position, *v, expand(table));
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "Envelope");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentAttack);
  intVarField_.emplace_back(position, *v, expand(attack));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentDecay);
  intVarField_.emplace_back(position, *v, expand(decay));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "Vibrato");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentVibratoDelay);
  intVarField_.emplace_back(position, *v, expand(vibrato_delay));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentVibrato);
  intVarField_.emplace_back(position, *v, expand(vibrato_amount));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "Sweep");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentSweepTime);
  intVarField_.emplace_back(position, *v, expand(sweep_time));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y++;
  v = instrument->FindVariable(FourCC::ChiptuneInstrumentSweepAmount);
  intVarField_.emplace_back(position, *v, expand(sweep_amount));
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
}