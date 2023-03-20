#include "Adapters/PICO/Utils/utils.h"
#include "CommandList.h"
#include "SIDInstrument.h"
#include "System/Console/Trace.h"
#include <string.h>

SIDInstrument::SIDInstrument() {

  strcpy(name_, "SID");

  Variable *v = new Variable("volume", DIP_VOLUME, 255);
  Insert(v);
  v = new Variable("table", DIP_TABLE, -1);
  Insert(v);
  v = new Variable("table automation", DIP_TABLEAUTO, false);
  Insert(v);

  sid_ = new cRSID(6581, false, 44100);
}

SIDInstrument::~SIDInstrument(){};

bool SIDInstrument::Init() {
  tableState_.Reset();
  return true;
};

void SIDInstrument::OnStart() {
  // SID config
  //sid_->set_chip_model(MOS6581);
  // Default config in constructor:
  // model: MOS6581
  // Samplingparams: 985248, SAMPLE_FAST, 44100
  // filter enabled
  //  sid_.enable_filter(false);
  //  delta_t_ = 0;
  // Set freq C3
  //  sid_.write(0x00, 0x93);
  sid_->Register[0] = 0x93;
  //  sid_.write(0x01, 0x08);
  sid_->Register[1] = 0x08;
  // Attack and decay to mid values
  //  sid_.write(0x05, 0x88);
  sid_->Register[5] = 0x88;
  // sustain 100%, sustain 50%
  //  sid_.write(0x06, 0xF8);
  sid_->Register[6] = 0xF8;
  // Set Triangle wave and open gate
  //  sid_.write(0x04, 0x11);
  sid_->Register[4] = 0x11;
  tableState_.Reset();
};

bool SIDInstrument::Start(int c, unsigned char note, bool retrigger) {
  playing_ = true;

  return true;
};

void SIDInstrument::Stop(int c) {
  playing_ = false;
};

bool SIDInstrument::Render(int channel, fixed *buffer, int size,
                           bool updateTick) {
  int start = micros();
  // clear the fixed point buffer

  SYS_MEMSET(buffer, 0, size * 2 * sizeof(fixed));
  /*
  // TEMP
  int bufSize = 32;
  //  short *buf = (short *)SYS_MALLOC(32 * sizeof(short));
  short buf[bufSize];

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
  for (int n = 0 ; n < size; n++) {
    // Have to calculate ASDRs somewhere here
    int output = sid_->cRSID_emulateWaves();
    buffer[n] = (fixed)output;
  }
  int time_taken = micros() - start;
  Trace::Log("RENDER", "SID Render took %ius (%i%% ts)", time_taken, (time_taken * 44100) / size / 10000);
  return false;
};

bool SIDInstrument::IsInitialized() {
  return true; // Always initialised
};

void SIDInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
};

const char *SIDInstrument::GetName() {
  return name_;
}

int SIDInstrument::GetTable() {
  Variable *v = FindVariable(DIP_TABLE);
  return v->GetInt();
};

bool SIDInstrument::GetTableAutomation() {
  Variable *v = FindVariable(DIP_TABLEAUTO);
  return v->GetBool();
};

void SIDInstrument::GetTableState(TableSaveState &state) {
};

void SIDInstrument::SetTableState(TableSaveState &state) {
};
