#include "TinysynthInstrument.h"

#include "I_Instrument.h"
#include "SRPUpdaters.h"
#include "SampleRenderingParams.h"

#include "Application/Model/Song.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "SoundSource.h"

TinysynthInstrument::TinysynthInstrument() { tinysynth_ = new TinySynth(); }

TinysynthInstrument::~TinysynthInstrument() {}

bool TinysynthInstrument::Init() {
  // TODO
  return false;
}

void TinysynthInstrument::OnStart() {
  // TODO
}

bool TinysynthInstrument::Start(int channel, unsigned char midinote,
                                bool cleanstart) {
  // set tinysynth defaults
  tinysynth_->set_defaults();
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

  tinysynth_->generateWaves(buffer, size);

  return true;
}

void TinysynthInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  // TODO
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
  // TODO
  return true;
}

bool TinysynthInstrument::IsEmpty() {
  // TODO
  return false;
};