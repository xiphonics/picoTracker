#include "OpalInstrument.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "I_Instrument.h"
#include "System/Console/Trace.h"
#include <string.h>

OpalInstrument::OpalInstrument() : I_Instrument(&variables_), opl_(44100) {

  // Reserve Observer
  ReserveObserver(1);
}

OpalInstrument::~OpalInstrument(){};

bool OpalInstrument::Init() {
  // Set chan 0 4-op
  opl_.Port(0x104, 0x1);

  return true;
};

void OpalInstrument::OnStart(){};

bool OpalInstrument::Start(int c, unsigned char note, bool retrigger) {
  int start = micros();

  // Frequency
  opl_.Port(0xA0, 0xe7);
  // Note on, block, hi freq
  opl_.Port(0xB0, 0x31);
  // Tremolo/Vibrato/Sustain/KSR/Multiplication
  opl_.Port(0x20, 0x00);
  opl_.Port(0x23, 0x01);
  opl_.Port(0x28, 0x06);
  opl_.Port(0x2B, 0x01);
  // Waveform
  opl_.Port(0xE0, 0x04);
  opl_.Port(0xE3, 0x03);
  opl_.Port(0xE8, 0x02);
  opl_.Port(0xEB, 0x02);
  // Key Scale Level/Output Level
  opl_.Port(0x40, 0x40);
  opl_.Port(0x43, 0x12);
  opl_.Port(0x48, 0x5c);
  opl_.Port(0x4B, 0x02);
  // Attack Rate/Decay Rate
  opl_.Port(0x60, 0x72);
  opl_.Port(0x63, 0x71);
  opl_.Port(0x68, 0x82);
  opl_.Port(0x6B, 0xf0);
  // Sustain Level/Release Rate
  opl_.Port(0x80, 0x86);
  opl_.Port(0x83, 0x94);
  opl_.Port(0x88, 0xbc);
  opl_.Port(0x8B, 0xbc);

  // enable left/right only for channel0
  opl_.Port(0xc0, 0x37);
  opl_.Port(0xc1, 0x00);
  opl_.Port(0xc2, 0x00);
  opl_.Port(0xc3, 0x00);
  opl_.Port(0xc4, 0x00);
  opl_.Port(0xc5, 0x00);
  opl_.Port(0xc6, 0x00);
  opl_.Port(0xc7, 0x00);
  opl_.Port(0xc8, 0x00);
  opl_.Port(0x1c0, 0x00);
  opl_.Port(0x1c1, 0x00);
  opl_.Port(0x1c2, 0x00);
  opl_.Port(0x1c3, 0x00);
  opl_.Port(0x1c4, 0x00);
  opl_.Port(0x1c5, 0x00);
  opl_.Port(0x1c6, 0x00);
  opl_.Port(0x1c7, 0x00);
  opl_.Port(0x1c8, 0x00);

  printf("Start took: %i us\n", micros() - start);

  return true;
};

void OpalInstrument::Stop(int c) { opl_.Port(0xB0, 0x11); };

bool OpalInstrument::Render(int channel, fixed *buffer, int size,

                            bool updateTick) {

  int start = micros();
  while (size--) {
    int16_t l, r;
    opl_.Sample(&l, &r);

    buffer[0] = l << 15;
    buffer[1] = r << 15;
    buffer += 2;
  }
  int took = micros() - start;
  printf("Render took: %i (%i ts)\n", took, took * 441 / size / 10000);
  return true;
};

bool OpalInstrument::IsInitialized() {
  return true; // Always initialised
};

void OpalInstrument::ProcessCommand(int channel, FourCC cc, ushort value){};

etl::string<24> OpalInstrument::GetName() { return "opal"; }

int OpalInstrument::GetTable() {
  //  Variable *v = FindVariable(MIP_TABLE);
  //  return v->GetInt();
  return 0;
};

bool OpalInstrument::GetTableAutomation() {
  //  Variable *v = FindVariable(MIP_TABLEAUTO);
  //  return v->GetBool();
  return 0;
};

void OpalInstrument::GetTableState(TableSaveState &state){

};

void OpalInstrument::SetTableState(TableSaveState &state){};
