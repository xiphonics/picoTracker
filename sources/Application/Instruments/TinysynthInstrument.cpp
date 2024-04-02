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
  lfo_ = new Variable("lfo", TXIP_LO, 0);
  insert(end(), lfo_);

  harmonicadsr_[0] = new Variable("h1adsr", TXIP_H1, 0);
  insert(end(), harmonicadsr_[0]);
  harmonicadsr_[1] = new Variable("h2adsr", TXIP_H2, 0);
  insert(end(), harmonicadsr_[1]);
  harmonicadsr_[2] = new Variable("h3adsr", TXIP_H3, 0);
  insert(end(), harmonicadsr_[2]);
  harmonicadsr_[3] = new Variable("h4adsr", TXIP_H4, 0);
  insert(end(), harmonicadsr_[3]);
  harmonicadsr_[4] = new Variable("h5adsr", TXIP_H5, 0);
  insert(end(), harmonicadsr_[4]);
  harmonicadsr_[5] = new Variable("h6adsr", TXIP_H6, 0);
  insert(end(), harmonicadsr_[5]);

  harmonicvol_[0] = new Variable("h1vol", TXIP_V1, 0);
  insert(end(), harmonicvol_[0]);
  harmonicvol_[1] = new Variable("h2vol", TXIP_V2, 0);
  insert(end(), harmonicvol_[1]);
  harmonicvol_[2] = new Variable("h3vol", TXIP_V3, 0);
  insert(end(), harmonicvol_[2]);
  harmonicvol_[3] = new Variable("h4vol", TXIP_V4, 0);
  insert(end(), harmonicvol_[3]);
  harmonicvol_[4] = new Variable("h5vol", TXIP_V5, 0);
  insert(end(), harmonicvol_[4]);
  harmonicvol_[5] = new Variable("h6vol", TXIP_V6, 0);
  insert(end(), harmonicvol_[5]);
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

  for (int i = 0; i < HARMONICS; i++) {

    tinysynth_env harmonic = {0, 0, 0, 0, 0, 0, 0};

    int adsrInt = harmonicadsr_[i]->GetInt();
    int adsrVol = harmonicvol_[i]->GetInt();
    harmonic.attack = (adsrInt >> 12) << 4;
    harmonic.decay = ((adsrInt >> 8) & 0xF) << 4;
    harmonic.sustain = ((adsrInt >> 4) & 0xF) << 4;
    harmonic.release = (adsrInt & 0xF) << 4;
    harmonic.amplitude = ((adsrVol >> 4) & 0xF) << 4;
    harmonic.type = (adsrVol & 0xF) << 4;

    printf("H1 Attack:%d ", harmonic.attack);
    printf("H1 Decay:%d ", harmonic.decay);
    printf("H1 Sustain:%d ", harmonic.sustain);
    printf("H1 Release:%d ", harmonic.release);
    printf("H1 Volume:%d ", harmonic.amplitude);
    printf("H1 Type:%d ", harmonic.type);
    printf("\n");

    tinysynth_->setEnvelopeConfig(i, harmonic);
  }

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
  const etl::string<24> name = "TINYSYNTH";
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