#include "TinysynthInstrument.h"

#include "I_Instrument.h"
#include "SRPUpdaters.h"
#include "SampleRenderingParams.h"

#include "Application/Model/Song.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "SoundSource.h"

TinysynthInstrument::TinysynthInstrument() {
  volume_ = new Variable("volume", TXIP_VOLUME, 0x80);
  insert(end(), volume_);

  harmonic1adsr_ = new Variable("h1adsr", TXIP_H1, 0);
  insert(end(), harmonic1adsr_);

  harmonic1vol_ = new Variable("h1vol", TXIP_V1, 0);
  insert(end(), harmonic1vol_);

  harmonic2adsr_ = new Variable("h2adsr", TXIP_H2, 0);
  insert(end(), harmonic1adsr_);

  harmonic2vol_ = new Variable("h2vol", TXIP_V2, 0);
  insert(end(), harmonic1vol_);
}

TinysynthInstrument::~TinysynthInstrument() {}

bool TinysynthInstrument::Init() {
  printf("Tinysynth Init!\n");
  tinysynth_ = new TinySynth();
  // set tinysynth defaults
  tinysynth_->set_defaults();
  return true;
}

void TinysynthInstrument::OnStart() {
  // TODO
}

bool TinysynthInstrument::Start(int channel, unsigned char midinote,
                                bool cleanstart) {

  tinysynth_->set_note(midinote);
  tinysynth_->envelope_gate(true);
  return true;
}

void TinysynthInstrument::Stop(int channel) {
  tinysynth_->envelope_gate(false);
}
// Size in samples
bool TinysynthInstrument::Render(int channel, fixed *buffer, int size,
                                 bool updateTick) {

  // clear the fixed point buffer
  SYS_MEMSET(buffer, 0, size * 2 * sizeof(fixed));

  // TODO pass in instrument volume
  tinysynth_->generateWaves(buffer, size, volume_->GetInt());

  return true;
}

void TinysynthInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  // printf("Tinysynth Process CMD:%d ch:%d val:%d", cc, channel, value);
}

int TinysynthInstrument::GetTable() {
  // TODO
  return 0;
}
bool TinysynthInstrument::GetTableAutomation() {
  // TODO
  return false;
}
void TinysynthInstrument::GetTableState(TableSaveState &state) {
  // TODO
}
void TinysynthInstrument::SetTableState(TableSaveState &state) {
  // TODO
}

void TinysynthInstrument::Purge() {
  // TODO
}

etl::string<24> TinysynthInstrument::GetName() {
  etl::string<24> name = "TINYSYNTH";
  return name;
};

void TinysynthInstrument::Update(Observable &o, I_ObservableData *d) {
  // TODO
}

bool TinysynthInstrument::IsInitialized() {
  return true; // Always initialised
}

bool TinysynthInstrument::IsEmpty() {
  // TODO
  return false;
};