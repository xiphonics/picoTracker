#include "SIDInstrument.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "CommandList.h"
#include "System/Console/Trace.h"
#include <string.h>

const char *sidWaveformText[DWF_LAST] = {"----", "T---", "-S--", "TS--", "--Q-",
                                         "T-Q-", "-SQ-", "TSQ-", "---N"};
const char *sidFilterModeText[DFM_LAST] = {"LP", "BP", "HP"};

SIDInstrument::SIDInstrument()
    : sid_(6581, false, 44100), v1pw_("V1PW", DIP_V1PW, 0x800),
      v1wf_("V1WF", DIP_V1WF, sidWaveformText, DWF_LAST, 0x1),
      v1sync_("V1SYNC", DIP_V1SYNC, false), v1gate_("V1GATE", DIP_V1GATE, true),
      v1ring_("V1RING", DIP_V1RING, false),
      v1adsr_("V1ADSR", DIP_V1ADSR, 0x2282), v1fon_("V1FON", DIP_V1FON, false),
      v2pw_("V2PW", DIP_V2PW, 0x800),
      v2wf_("V2WF", DIP_V2WF, sidWaveformText, DWF_LAST, 0x0),
      v2sync_("V2SYNC", DIP_V2SYNC, false), v2gate_("V2GATE", DIP_V2GATE, true),
      v2ring_("V2RING", DIP_V2RING, false),
      v2adsr_("V2ADSR", DIP_V2ADSR, 0x2282), v2fon_("V2FON", DIP_V2FON, false),
      v3pw_("V3PW", DIP_V3PW, 0x800),
      v3wf_("V3WF", DIP_V3WF, sidWaveformText, DWF_LAST, 0x0),
      v3sync_("V3SYNC", DIP_V3SYNC, false), v3gate_("V3GATE", DIP_V3GATE, true),
      v3ring_("V3RING", DIP_V3RING, false),
      v3adsr_("V3ADSR", DIP_V3ADSR, 0x2282), v3fon_("V3FON", DIP_V3FON, false),
      v3off_("V3OFF", DIP_V3OFF, false), fltcut_("FILTCUT", DIP_FILTCUT, 0x1FF),
      fltres_("RES", DIP_RES, 0x0),
      fltmode_("FMODE", DIP_FMODE, sidFilterModeText, DFM_LAST, 0x0),
      vol_("DIP_VOLUME", DIP_VOLUME, 0xF) {

  strcpy(name_, "SID");

  Variable *v = new Variable("table", DIP_TABLE, -1);
  Insert(v);
  v = new Variable("table automation", DIP_TABLEAUTO, false);
  Insert(v);

  Insert(v1pw_);
  Insert(v1wf_);
  Insert(v1sync_);
  Insert(v1gate_);
  Insert(v1ring_);
  //  v1adsr_ = new Variable("V1ADSR", DIP_V1ADSR,
  //                         0x2282); // TODO: What's a good default?
  Insert(v1adsr_);
  Insert(v1fon_);
  Insert(v2pw_);
  Insert(v2wf_);
  Insert(v2sync_);
  Insert(v2gate_);
  Insert(v2ring_);
  Insert(v2adsr_);
  Insert(v2fon_);

  Insert(v3pw_);
  Insert(v3wf_);
  Insert(v3sync_);
  Insert(v3gate_);
  Insert(v3ring_);
  Insert(v3adsr_);
  Insert(v3fon_);
  Insert(v3off_);

  //  fltcut_ = new Variable("FILTCUT", DIP_FILTCUT,
  //                         0x1FF); // TODO: what's a
  //                         good default?
  Insert(fltcut_);
  Insert(fltres_);
  //  fltmode_ = new Variable("FMODE", DIP_FMODE,
  //  sidFilterModeText, DFM_LAST,
  //                          0x0); // TODO: What's a
  //                          good default?
  Insert(fltmode_);
  Insert(vol_);
}

SIDInstrument::~SIDInstrument(){};

bool SIDInstrument::Init() {
  tableState_.Reset();
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
  playing_ = true;
  printf("Playing note: %i\n", note);
  //  Shift = -24;
  sid_.Register[0] = sid_notes[note - 24] & 0xFF; // V1 Freq Lo
  sid_.Register[1] = sid_notes[note - 24] >> 8;   // V1 Freq Hi
  sid_.Register[2] = v1pw_.GetInt() & 0xFF;       // V1 PW Lo
  sid_.Register[3] = v1pw_.GetInt() >> 8;         // V1 PW Hi
  sid_.Register[4] = v1wf_.GetInt() << 4 | v1ring_.GetInt() << 2 |
                     v1sync_.GetInt() << 1 | v1gate_.GetInt(); // V1 Control Reg
  sid_.Register[5] = v1adsr_.GetInt() >> 8;   // V1 Attack/Decay
  sid_.Register[6] = v1adsr_.GetInt() & 0xFF; // V1 Sustain/Release
  sid_.Register[7] = 0;                       // V2 Freq Lo
  sid_.Register[8] = 0;                       // V2 Freq Hi
  sid_.Register[9] = v2pw_.GetInt() & 0xFF;   // V2 PW Lo
  sid_.Register[10] = v2pw_.GetInt() >> 8;    // V2 PW Hi
  sid_.Register[11] = v2wf_.GetInt() << 4 | v2ring_.GetInt() << 2 |
                      v2sync_.GetInt() << 1 |
                      v2gate_.GetInt();        // V2 Control Reg
  sid_.Register[12] = v2adsr_.GetInt() >> 8;   // V2 Attack/Decay
  sid_.Register[13] = v3adsr_.GetInt() & 0xFF; // V2 Sustain/Release
  sid_.Register[14] = 0;                       // V3 Freq Lo
  sid_.Register[15] = 0;                       // V3 Freq Hi
  sid_.Register[16] = v3pw_.GetInt() & 0xFF;   // V3 PW Lo
  sid_.Register[17] = v2pw_.GetInt() >> 8;     // V3 PW Hi
  sid_.Register[18] = v3wf_.GetInt() << 4 | v3ring_.GetInt() << 2 |
                      v3sync_.GetInt() << 1 |
                      v3gate_.GetInt();        // V3 Control Reg
  sid_.Register[19] = v3adsr_.GetInt() >> 8;   // V3 Attack/Decay
  sid_.Register[20] = v3adsr_.GetInt() & 0xFF; // V3 Sustain/Release
  sid_.Register[21] = fltcut_.GetInt() >> 8;   // Filter Cut lo  TODO: MAL
  sid_.Register[22] = fltcut_.GetInt() & 0xFF; // Filter Cut Hi  TODO: MAL
  sid_.Register[23] = fltres_.GetInt() << 4 | v3fon_.GetInt() << 2 |
                      v2fon_.GetInt() << 1 | v1fon_.GetInt(); // Filter Res/Filt

  int8_t mode = 0;
  switch (fltmode_.GetInt()) {
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
  sid_.Register[24] =
      v3off_.GetInt() << 7 | mode << 4 | vol_.GetInt(); // Filter Mode/Vol

  for (int i = 0; i <= 24; i++) {
    printf("Register %i value: -  0x%2.2X\n", i, sid_.Register[i]);
  }
  return true;
};

void SIDInstrument::Stop(int c) { playing_ = false; };

bool SIDInstrument::Render(int channel, fixed *buffer, int size,
                           bool updateTick) {
  //  int start = micros();
  // clear the fixed point buffer

  SYS_MEMSET(buffer, 0, size * 2 * sizeof(fixed));
  /*
  // TEMP
  int bufSize = 32;
  //  short *buf = (short *)SYS_MALLOC(32 *
  sizeof(short)); short buf[bufSize];

  int samplesElapsed = 0;
  int bufindex = 0;
  //  cyclesPerSample = sid_.getCyclesPerSample();

  int delta_t = 10000000; // large enough value
  while (samplesElapsed < size) {
    int samplesToCalc;
    if ((size - samplesElapsed) / bufSize > 1) {
      samplesToCalc = bufSize;
    } else {
      samplesToCalc = bufSize + samplesElapsed - size;
    }
    sid_.clock(delta_t, buf, samplesToCalc, 1);
    for (int s = 0; s < bufSize; s++) {
      buffer[s] = (fixed)buf[s];
    }
    samplesElapsed += bufSize;
    }*/
  for (int n = 0; n < size; n++) {
    // Have to calculate ASDRs somewhere here
    sid_.cRSID_emulateADSRs(1);
    int output = sid_.cRSID_emulateWaves();
    buffer[2 * n] = (fixed)output * 65536;     // L
    buffer[2 * n + 1] = (fixed)output * 65536; // R
  }
  //  int time_taken = micros() - start;
  //  Trace::Log("RENDER", "SID Render took %ius (%i%%
  //  ts)", time_taken,
  //             (time_taken * 44100) / size / 10000);
  return true;
};

bool SIDInstrument::IsInitialized() {
  return true; // Always initialised
};

void SIDInstrument::ProcessCommand(int channel, FourCC cc, ushort value){};

const char *SIDInstrument::GetName() { return name_; }

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
