/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SIDInstrument.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "I_Instrument.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include <string.h>

const char *sidWaveformText[DWF_LAST] = {"----", "T---", "-S--", "TS--", "--Q-",
                                         "T-Q-", "-SQ-", "TSQ-", "---N"};
const char *sidFilterModeText[DFM_LAST] = {"LP", "BP", "HP", "Notch"};

cRSID SIDInstrument::sid1_(44100);
SIDInstrument *SIDInstrument::SID1RenderMaster = 0;
cRSID SIDInstrument::sid2_(44100);
SIDInstrument *SIDInstrument::SID2RenderMaster = 0;

Variable SIDInstrument::fltcut1_(FourCC::SIDInstrument1FilterCut, 0x1FF);
Variable SIDInstrument::fltres1_(FourCC::SIDInstrument1FilterResonance, 0x0);
Variable SIDInstrument::fltmode1_(FourCC::SIDInstrument1FilterMode,
                                  sidFilterModeText, DFM_LAST, 0x0);
Variable SIDInstrument::vol1_(FourCC::SIDInstrument1Volume, 0xF);

Variable SIDInstrument::fltcut2_(FourCC::SIDInstrument2FilterCut, 0x1FF);
Variable SIDInstrument::fltres2_(FourCC::SIDInstrument2FilterResonance, 0x0);
Variable SIDInstrument::fltmode2_(FourCC::SIDInstrument2FilterMode,
                                  sidFilterModeText, DFM_LAST, 0x0);
Variable SIDInstrument::vol2_(FourCC::SIDInstrument2Volume, 0xF);

SIDInstrument::SIDInstrument(SIDInstrumentInstance chip)
    : I_Instrument(&variables_), chip_(chip),
      vpw_(FourCC::SIDInstrumentPulseWidth, 0x800),
      vwf_(FourCC::SIDInstrumentWaveform, sidWaveformText, DWF_LAST, 0x1),
      vsync_(FourCC::SIDInstrumentVSync, false),
      vring_(FourCC::SIDInstrumentRingModulator, false),
      vadsr_(FourCC::SIDInstrumentADSR, 0x2282),
      vfon_(FourCC::SIDInstrumentFilterOn, false),
      table_(FourCC::SIDInstrumentTable, -1),
      tableAuto_(FourCC::SIDInstrumentTableAutomation, false),
      osc_(FourCC::SIDInstrumentOSCNumber, 0) {

  // name_ is now an etl::string in the base class, not a Variable
  variables_.insert(variables_.end(), &vpw_);
  variables_.insert(variables_.end(), &vwf_);
  variables_.insert(variables_.end(), &vsync_);
  variables_.insert(variables_.end(), &vring_);
  variables_.insert(variables_.end(), &vadsr_);
  variables_.insert(variables_.end(), &vfon_);
  variables_.insert(variables_.end(), &table_);
  variables_.insert(variables_.end(), &tableAuto_);
  variables_.insert(variables_.end(), &osc_);
  variables_.insert(variables_.end(), &fltcut1_);
  variables_.insert(variables_.end(), &fltres1_);
  variables_.insert(variables_.end(), &fltmode1_);
  variables_.insert(variables_.end(), &vol1_);

  variables_.insert(variables_.end(), &fltcut2_);
  variables_.insert(variables_.end(), &fltres2_);
  variables_.insert(variables_.end(), &fltmode2_);
  variables_.insert(variables_.end(), &vol2_);
}

SIDInstrument::~SIDInstrument(){};

bool SIDInstrument::Init() {
  tableState_.Reset();

  Trace::Debug("SID instrument chip is %i and osc is %i", chip_, GetOsc());
  switch (chip_) {
  case 1:
    sid_ = &sid1_;
    fltcut_ = &fltcut1_;
    fltres_ = &fltres1_;
    fltmode_ = &fltmode1_;
    vol_ = &vol1_;
    break;
  case 2:
    sid_ = &sid2_;
    fltcut_ = &fltcut2_;
    fltres_ = &fltres2_;
    fltmode_ = &fltmode2_;
    vol_ = &vol2_;
    break;
  default:
    return false;
  }

  return true;
};

void SIDInstrument::OnStart() {
  tableState_.Reset();
  int osc = GetOsc();
  sid_->cRSID_resetADSR(osc);
};

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                   \
  ((byte) & 0x80 ? '1' : '0'), ((byte) & 0x40 ? '1' : '0'),                    \
      ((byte) & 0x20 ? '1' : '0'), ((byte) & 0x10 ? '1' : '0'),                \
      ((byte) & 0x08 ? '1' : '0'), ((byte) & 0x04 ? '1' : '0'),                \
      ((byte) & 0x02 ? '1' : '0'), ((byte) & 0x01 ? '1' : '0')

bool SIDInstrument::Start(int c, unsigned char note, bool retrigger) {
  Trace::Debug("Retrigger: %i", retrigger);
  gate_ = retrigger;
  // Select master render instrument
  // At each row of the sequencer we call start for each instrument in
  // the channel. With this we are ensuring that the only instrument that
  // renders audio is the last in use per SID chip (to ensure that all settings
  // are set before rendering and to only render once per chip)
  // I *think* that this could be done only on retrigger and would work fine
  switch (chip_) {
  case SID1:
    if (SID1RenderMaster) {
      SID1RenderMaster->SetRender(false);
      Trace::Debug("Previous renderer for SID1 was %s",
                   SID1RenderMaster->GetName().c_str());
    }
    SID1RenderMaster = this;
    SID1RenderMaster->SetRender(true);
    Trace::Debug("New renderer for SID1 is %s",
                 SID1RenderMaster->GetName().c_str());
    break;
  case SID2:
    if (SID2RenderMaster) {
      SID2RenderMaster->SetRender(false);
      Trace::Debug("Previous renderer for SID2 was %s",
                   SID2RenderMaster->GetName().c_str());
    }
    SID2RenderMaster = this;
    SID2RenderMaster->SetRender(true);
    Trace::Debug("New renderer for SID2 is %s",
                 SID2RenderMaster->GetName().c_str());
    break;
  }

  int osc = GetOsc();

  sid_->Register[0 + osc * 7] = sid_notes[note - 24] & 0xFF; // V1 Freq Lo
  sid_->Register[1 + osc * 7] = sid_notes[note - 24] >> 8;   // V1 Freq Hi
  sid_->Register[2 + osc * 7] = vpw_.GetInt() & 0xFF;        // V1 PW Lo
  sid_->Register[3 + osc * 7] = vpw_.GetInt() >> 8;          // V1 PW Hi
  sid_->Register[4 + osc * 7] = vwf_.GetInt() << 4 | vring_.GetInt() << 2 |
                                vsync_.GetInt() << 1 |
                                (int)gate_;             // V1 Control Reg
  sid_->Register[5 + osc * 7] = vadsr_.GetInt() >> 8;   // V1 Attack/Decay
  sid_->Register[6 + osc * 7] = vadsr_.GetInt() & 0xFF; // V1 Sustain/Release

  // filter settings
  sid_->Register[21] = fltcut_->GetInt() & 0x7; // Filter Cut lo
  sid_->Register[22] = fltcut_->GetInt() >> 3;  // Filter Cut Hi

  // on start for each instrument it sets its own filter on bit in this register
  //  we need to clear filter resonance and the current oscillator's filter on
  //  bit while preserving the filter on bits of the other two oscillators for
  //  this chip
  sid_->Register[23] = (sid_->Register[23] & 0xF & ~(1 << osc)) |
                       fltres_->GetInt() << 4 | // filter resonance
                       vfon_.GetInt() << osc;   // filter on bit for this osc

  int8_t mode = 0;
  switch (fltmode_->GetInt()) {
  case DFM_LP:
    mode = 1;
    break;
  case DFM_BP:
    mode = 2;
    break;
  case DFM_HP:
    mode = 4;
    break;
  case DFM_NOTCH:
    mode = 5;
    break;
  }
  // TODO: implement v3off
  //  sid_->Register[24] =
  //      v3off_.GetInt() << 7 | mode << 4 | vol_->GetInt(); // Filter Mode/Vol

  sid_->Register[24] = 0 << 7 | mode << 4 | vol_->GetInt(); // Filter Mode / Vol

  //  for (int n = 0; n < 29; n++) {
  //    printf("Register %i value: 0x%x (0b" BYTE_TO_BINARY_PATTERN ")",
  //    n,
  //           sid_->Register[n], BYTE_TO_BINARY(sid_->Register[n]));
  //  }

  playing_ = true;

  return true;
};

void SIDInstrument::Stop(int c) {
  playing_ = false;
  int osc = GetOsc();
  sid_->Register[4 + osc * 7] &= ~1; // Set gate bit off
  gate_ = false;
};

bool SIDInstrument::Render(int channel, fixed *buffer, int size,
                           bool updateTick) {
  if (playing_ and render_) {

    // clear the fixed point buffer
    memset(buffer, 0, size * 2 * sizeof(fixed));

    sid_->cRSID_emulateWavesBuffer(buffer, size);

    return true;
  }
  return false;
};

bool SIDInstrument::IsInitialized() {
  return true; // Always initialised
};

void SIDInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  switch (cc) {
  case FourCC::InstrumentCommandGateOff:
    int osc = GetOsc();
    sid_->Register[4 + osc * 7] &= ~1; // Set gate bit off
    gate_ = false;
    break;
  }
};

etl::string<MAX_INSTRUMENT_NAME_LENGTH> SIDInstrument::GetName() {
  // first check if the name_ string has been explicitly set
  if (!name_.empty()) {
    return name_;
  }
  // otherwise return the default name for this instrument type
  return etl::string<MAX_INSTRUMENT_NAME_LENGTH>(InstrumentTypeNames[IT_SID]);
}

int SIDInstrument::GetTable() {
  Variable *v = FindVariable(FourCC::SIDInstrumentTable);
  return v->GetInt();
};

bool SIDInstrument::GetTableAutomation() {
  Variable *v = FindVariable(FourCC::SIDInstrumentTableAutomation);
  return v->GetBool();
};

void SIDInstrument::GetTableState(TableSaveState &state){};

void SIDInstrument::SetTableState(TableSaveState &state){};
