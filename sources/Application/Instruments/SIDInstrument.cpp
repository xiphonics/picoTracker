#include "SIDInstrument.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include <string.h>

const char *sidWaveformText[DWF_LAST] = {"----", "T---", "-S--", "TS--", "--Q-",
                                         "T-Q-", "-SQ-", "TSQ-", "---N"};
const char *sidFilterModeText[DFM_LAST] = {"LP", "BP", "HP"};

cRSID SIDInstrument::sid1_(6581, false, 44100);
SIDInstrument *SIDInstrument::SID1RenderMaster = 0;
cRSID SIDInstrument::sid2_(6581, false, 44100);
SIDInstrument *SIDInstrument::SID2RenderMaster = 0;
// bool SIDInstrument::rendered1_ = false;

Variable SIDInstrument::vwf1_("VWF1", DIP_VWF1, sidWaveformText, DWF_LAST, 0x1);
Variable SIDInstrument::vwf2_("VWF2", DIP_VWF2, sidWaveformText, DWF_LAST, 0x1);

Variable SIDInstrument::fltcut1_("FILTCUT1", DIP_FILTCUT1, 0x1FF);
Variable SIDInstrument::fltres1_("RES1", DIP_RES1, 0x0);
Variable SIDInstrument::fltmode1_("FMODE1", DIP_FMODE1, sidFilterModeText,
                                  DFM_LAST, 0x0);
Variable SIDInstrument::vol1_("DIP_VOLUME1", DIP_VOLUME1, 0xF);

Variable SIDInstrument::fltcut2_("FILTCUT2", DIP_FILTCUT2, 0x1FF);
Variable SIDInstrument::fltres2_("RES2", DIP_RES2, 0x0);
Variable SIDInstrument::fltmode2_("FMODE2", DIP_FMODE2, sidFilterModeText,
                                  DFM_LAST, 0x0);
Variable SIDInstrument::vol2_("DIP_VOLUME2", DIP_VOLUME2, 0xF);

SIDInstrument::SIDInstrument(SIDInstrumentInstance chip, int osc)
    : chip_(chip), osc_(osc), vpw_("VPW", DIP_VPW, 0x800),
      vsync_("VSYNC", DIP_VSYNC, false), vring_("VRING", DIP_VRING, false),
      vadsr_("VADSR", DIP_VADSR, 0x2282), vfon_("VFON", DIP_VFON, false) {

  name_ = "SID #";
  etl::string<1> num;
  etl::format_spec format;
  etl::to_string((int)chip_, num, format);
  name_ += num;
  name_ += " OSC #";
  etl::to_string(osc_, num, format);
  name_ += num;

  Variable *v = new Variable("table", DIP_TABLE, -1);
  insert(end(), v);
  v = new Variable("table automation", DIP_TABLEAUTO, false);
  insert(end(), v);

  insert(end(), &vpw_);
  insert(end(), &vwf1_);
  insert(end(), &vsync_);
  insert(end(), &vring_);
  //  v1adsr_ = new Variable("V1ADSR", DIP_V1ADSR,
  //                         0x2282); // TODO: What's a good default?
  insert(end(), &vadsr_);
  insert(end(), &vfon_);

  //  fltcut_ = new Variable("FILTCUT", DIP_FILTCUT,
  //                         0x1FF); // TODO: what's a
  //                         good default?
  insert(end(), &fltcut1_);
  insert(end(), &fltres1_);
  //  fltmode_ = new Variable("FMODE", DIP_FMODE,
  //  sidFilterModeText, DFM_LAST,
  //                          0x0); // TODO: What's a
  //                          good default?
  insert(end(), &fltmode1_);
  insert(end(), &vol1_);

  insert(end(), &fltcut2_);
  insert(end(), &fltres2_);
  //  fltmode_ = new Variable("FMODE", DIP_FMODE,
  //  sidFilterModeText, DFM_LAST,
  //                          0x0); // TODO: What's a
  //                          good default?
  insert(end(), &fltmode2_);
  insert(end(), &vol2_);
}

SIDInstrument::~SIDInstrument(){};

bool SIDInstrument::Init() {
  tableState_.Reset();

  printf("SID instrument chip is %i and osc is %i\n", chip_, osc_);
  switch (chip_) {
  case 1:
    sid_ = &sid1_;
    vwf_ = &vwf1_;
    fltcut_ = &fltcut1_;
    fltres_ = &fltres1_;
    fltmode_ = &fltmode1_;
    vol_ = &vol1_;
    break;
  case 2:
    sid_ = &sid2_;
    vwf_ = &vwf2_;
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
  // SID config
  // sid_->set_chip_model(MOS6581);
  // Default config in constructor:
  // model: MOS6581
  // Samplingparams: 985248, SAMPLE_FAST, 44100
  // filter enabled
  //  sid_.enable_filter(false);
  //  delta_t_ = 0;
  // Set freq C3
  //  sid_.write(0x00, 0x93);
  //*  sid_->Register[0] = 0x93;
  //  sid_.write(0x01, 0x08);
  //*  sid_->Register[1] = 0x08;
  // Attack and decay to mid values
  //  sid_.write(0x05, 0x88);
  ///////  sid_->Register[5] = 0x88;
  // sustain 100%, sustain 50%
  //  sid_.write(0x06, 0xF8);
  //////  sid_->Register[6] = 0xF8;
  // Set Triangle wave and open gate
  //  sid_.write(0x04, 0x11);
  /////////  sid_->Register[4] = 0x11;

  // Main volume
  ///////  sid_->Register[24] = 0x0F;
  tableState_.Reset();
};

bool SIDInstrument::Start(int c, unsigned char note, bool retrigger) {
  printf("Retrigger: %i\n", retrigger);
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
      printf("Previous renderer for SID1 was %s\n",
             SID1RenderMaster->GetName().c_str());
    }
    SID1RenderMaster = this;
    SID1RenderMaster->SetRender(true);
    printf("New renderer for SID1 is %s\n",
           SID1RenderMaster->GetName().c_str());
    break;
  case SID2:
    if (SID2RenderMaster) {
      SID2RenderMaster->SetRender(false);
      printf("Previous renderer for SID2 was %s\n",
             SID2RenderMaster->GetName().c_str());
    }
    SID2RenderMaster = this;
    SID2RenderMaster->SetRender(true);
    printf("New renderer for SID2 is %s\n",
           SID2RenderMaster->GetName().c_str());
    break;
  }

  sid_->Register[0 + osc_ * 7] = sid_notes[note - 24] & 0xFF; // V1 Freq Lo
  sid_->Register[1 + osc_ * 7] = sid_notes[note - 24] >> 8;   // V1 Freq Hi
  sid_->Register[2 + osc_ * 7] = vpw_.GetInt() & 0xFF;        // V1 PW Lo
  sid_->Register[3 + osc_ * 7] = vpw_.GetInt() >> 8;          // V1 PW Hi
  sid_->Register[4 + osc_ * 7] = vwf_->GetInt() << 4 | vring_.GetInt() << 2 |
                                 vsync_.GetInt() << 1 |
                                 (int)gate_;             // V1 Control Reg
  sid_->Register[5 + osc_ * 7] = vadsr_.GetInt() >> 8;   // V1 Attack/Decay
  sid_->Register[6 + osc_ * 7] = vadsr_.GetInt() & 0xFF; // V1 Sustain/Release

  // filter settings
  sid_->Register[21] = fltcut_->GetInt() >> 8;   // Filter Cut lo  TODO: MAL
  sid_->Register[22] = fltcut_->GetInt() & 0xFF; // Filter Cut Hi  TODO: MAL

  // on start for each instrument it sets it's own filter in this register
  sid_->Register[23] = sid_->Register[23] | fltres_->GetInt() << 4 |
                       vfon_.GetInt() << osc_; // Filter Res/Filt

  int8_t mode = 0;
  switch (fltmode_->GetInt()) {
  case 0:
    mode = 1;
    break;
  case 1:
    mode = 2;
    break;
  case 2:
    mode = 4;
    break;
  }
  // TODO: implement v3off
  //  sid->Register[24] =
  //      v3off_.GetInt() << 7 | mode << 4 | vol->GetInt(); // Filter Mode/Vol

  sid_->Register[24] = 0 << 7 | mode << 4 | vol_->GetInt(); // Filter Mode / Vol

  playing_ = true;

  for (int i = 0; i <= 24; i++) {
    printf("Register %i value: -  0x%2.2X\n", i, sid_->Register[i]);
  }
  return true;
};

void SIDInstrument::Stop(int c) { playing_ = false; };

bool SIDInstrument::Render(int channel, fixed *buffer, int size,
                           bool updateTick) {
  int start = micros();

  if (playing_ and render_) {

    // clear the fixed point buffer
    SYS_MEMSET(buffer, 0, size * 2 * sizeof(fixed));

    for (int n = 0; n < size; n++) {
      // Have to calculate ASDRs somewhere here
      sid_->cRSID_emulateADSRs(1);
      int output = sid_->cRSID_emulateWaves();
      buffer[2 * n] = (fixed)output << 15;     // L
      buffer[2 * n + 1] = (fixed)output << 15; // R
    }
    int time_taken = micros() - start;
    Trace::Log("RENDER", "SID Render took %ius (%i%%ts)", time_taken,
               (time_taken * 44100) / size / 10000);
    return true;
  }
  int time_taken = micros() - start;
  Trace::Log("RENDER", "SID Render took %ius (%i%%ts)", time_taken,
             (time_taken * 44100) / size / 10000);
  return false;
};

bool SIDInstrument::IsInitialized() {
  return true; // Always initialised
};

void SIDInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  switch (cc) {
  case I_CMD_GTOF:
    sid_->Register[4 + osc_ * 7] &= ~1; // Set gate bit off
    break;
  }
};

etl::string<24> SIDInstrument::GetName() { return name_; }

int SIDInstrument::GetTable() {
  Variable *v = FindVariable(DIP_TABLE);
  return v->GetInt();
};

bool SIDInstrument::GetTableAutomation() {
  Variable *v = FindVariable(DIP_TABLEAUTO);
  return v->GetBool();
};

void SIDInstrument::GetTableState(TableSaveState &state){};

void SIDInstrument::SetTableState(TableSaveState &state){};
