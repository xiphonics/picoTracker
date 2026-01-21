void InstrumentView::fillGameBoyParameters() {
  int i = viewData_->currentInstrumentID_;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
  I_Instrument *instr = bank->GetInstrument(i);
  GameBoyInstrument *instrument = (GameBoyInstrument *)instr;
  GUIPoint position = GetAnchor();

  // min max x y visOffset

  // extra y spacing to allow for gap between export/import and parameters
  position._y += 2;
  Variable *v = instrument->FindVariable(FourCC::GameBoyInstrumentWaveform);
  int max = GB_NUM_WAVEFORMS;
  intVarField_.emplace_back(position, *v, "Waveform:   %s", 0, max - 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentTranspose);
  intVarField_.emplace_back(position, *v, "Transpose:  %+d", 0, 48, 1, 12, -24);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentLevel);
  intVarField_.emplace_back(position, *v, "Level:      %02X", 0, 255, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentBurst);
  intVarOffField_.emplace_back(position, *v, "Burst:      %02X", 1, 255, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentArpSpeed);
  intVarField_.emplace_back(position, *v, "Arp Speed:  %02X", 1, 32, 1, 8);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentLength);
  intVarOffField_.emplace_back(position, *v, "Length:     %02X", 1, 255, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarOffField_.rbegin()));

  position._y += 1;
  max = TABLE_COUNT - 1;
  const char *text = "Table:      %02X";
  v = instrument->FindVariable(FourCC::GameBoyInstrumentTable);
  intVarField_.emplace_back(position, *v, text, 0x00, max, 1, 0x10);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "Envelope");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentAttack);
  intVarField_.emplace_back(position, *v, " " char_border_single_verticalRight_s char_border_single_horizontal_s " Attack: %02X", 0, 255, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentDecay);
  intVarField_.emplace_back(position, *v, " " char_border_single_bottomLeft_s char_border_single_horizontal_s " Decay:  %02X", 0, 255, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "Vibrato");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentVibrato);
  intVarField_.emplace_back(position, *v, " " char_border_single_verticalRight_s char_border_single_horizontal_s " Amount: %02X", 0, 255, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentVibratoDelay);
  intVarField_.emplace_back(position, *v, " " char_border_single_bottomLeft_s char_border_single_horizontal_s " Delay:  %02X", 0, 255, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  staticField_.emplace_back(position, "Sweep");
  fieldList_.insert(fieldList_.end(), &(*staticField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentSweepTime);
  intVarField_.emplace_back(position, *v, " " char_border_single_verticalRight_s char_border_single_horizontal_s " Length: %02X", 0, 255, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 1;
  v = instrument->FindVariable(FourCC::GameBoyInstrumentSweepAmount);
  intVarField_.emplace_back(position, *v, " " char_border_single_bottomLeft_s char_border_single_horizontal_s " Amount: %03d", -127, 127, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
}