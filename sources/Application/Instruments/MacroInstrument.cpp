#include "MacroInstrument.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "Application/Instruments/Filters.h"
#include "Application/Model/Table.h"
#include "Application/Player/PlayerMixer.h" // For MIX_BUFFER_SIZE.. kick out pls
#include "Application/Player/SyncMaster.h"
#include "CommandList.h"
#include "I_Instrument.h"
#include "Services/Audio/Audio.h"
#include "System/Console/Trace.h"
#include "System/io/Status.h"
#include "pico/rand.h"
#include <assert.h>

MacroInstrument::MacroInstrument()
    : I_Instrument(&variables_),
      shape_("shape", FourCC::MacroInstrumentShape, braids::algo_values,
             braids::MACRO_OSC_SHAPE_LAST - 2, 0),
      timbre_("timbre", FourCC::MacroInstrmentTimbre, 0x7f),
      color_("color", FourCC::MacroInstrumentColor, 0x7f),
      attack_("Attack", FourCC::MacroInstrumentAttack, 0),
      decay_("Decay", FourCC::MacroInstrumentDecay, 0x05),
      signature_("Signature", FourCC::MacroInstrumentSignature, 0) {

  running_ = false;

  variables_.insert(variables_.end(), &shape_);
  variables_.insert(variables_.end(), &timbre_);
  variables_.insert(variables_.end(), &color_);
  variables_.insert(variables_.end(), &attack_);
  variables_.insert(variables_.end(), &decay_);
  variables_.insert(variables_.end(), &signature_);
}

MacroInstrument::~MacroInstrument() {}

bool MacroInstrument::Init() {
  printf("init!\n");

  osc_.Init();
  // we shouldn't need a quantizer
  //  quantizer_.Init();
  envelope_.Init();
  ws_.Init(get_rand_32());
  jitter_source_.Init();

  return true;
}

void MacroInstrument::OnStart(){/* tableState_.Reset();*/};

bool MacroInstrument::Start(int channel, unsigned char midinote,
                            bool cleanstart) {
  Variable *vShape = FindVariable(FourCC::MacroInstrumentShape);
  osc_shape_ = static_cast<braids::MacroOscillatorShape>(vShape->GetInt());

  running_ = true;

  // used convention: pT midi #81 == A4 vs braids #69
  int32_t pitch = 128 * (midinote - 12);

  if (pitch > 16383) {
    pitch = 16383;
  } else if (pitch < 0) {
    pitch = 0;
  }
  osc_.set_pitch(pitch);

  // start is a trigger, reset data
  osc_.Strike();
  envelope_.Trigger(braids::ENV_SEGMENT_ATTACK);
  return true;
}

void MacroInstrument::Stop(int channel) { running_ = false; }

// Size in samples
bool MacroInstrument::Render(int channel, fixed *buffer, int size,
                             bool updateTick) {
  //  int start = micros();

  // clear the fixed point buffer
  SYS_MEMSET(buffer, 0, size * 2 * sizeof(fixed));

  envelope_.Update(attack_.GetInt(), decay_.GetInt());
  uint32_t ad_value = envelope_.Render();

  osc_.set_shape(osc_shape_);
  osc_.set_parameters(timbre_.GetInt() * 128,
                      color_.GetInt() * 128); // timbre and color

  uint32_t block_size = 24;
  uint32_t num_blocks = size / block_size;
  remain_ = size % block_size;

  uint8_t sync_buffer[24] = {};
  int16_t render_buffer[24];
  for (uint32_t i = 0; i < num_blocks; i++) {
    osc_.Render(sync_buffer, render_buffer, block_size);
    uint16_t signature = signature_.GetInt() * signature_.GetInt() * 4095;
    for (uint32_t j = 0; j < block_size; j++) {
      int16_t sample = render_buffer[j] * gain_lp_ >> 16;
      gain_lp_ += (ad_value - gain_lp_) >> 4;

      int16_t warped = ws_.Transform(sample);
      int16_t mix = braids::Mix(sample, warped, signature) / 4.0;
      int32_t fp_sample = i2fp(mix);
      buffer[2 * (i * block_size + j)] = fp_sample;
      buffer[2 * (i * block_size + j) + 1] = fp_sample;
    }
  }

  osc_.Render(sync_buffer, render_buffer, remain_);
  uint16_t signature = signature_.GetInt() * signature_.GetInt() * 4095;
  for (uint32_t j = 0; j < remain_; j++) {
    int16_t sample = render_buffer[j] * gain_lp_ >> 16;
    gain_lp_ += (ad_value - gain_lp_) >> 4;
    int16_t warped = ws_.Transform(sample);

    int16_t mix = braids::Mix(sample, warped, signature) / 4.0;
    int32_t fp_sample = i2fp(mix);
    buffer[2 * (num_blocks * block_size + j)] = fp_sample;
    buffer[2 * (num_blocks * block_size + j) + 1] = fp_sample;
  }

  return true;
};

bool MacroInstrument::IsInitialized() { /*return (source_ != 0); */
  return true;
};

void MacroInstrument::Update(Observable &o, I_ObservableData *d){};

void MacroInstrument::ProcessCommand(int channel, FourCC cc, ushort value){};

etl::string<24> MacroInstrument::GetName() {
  Variable *v = FindVariable(FourCC::MacroInstrumentShape);
  return v->GetString();
};

void MacroInstrument::Purge(){};

bool MacroInstrument::IsEmpty() { return false; };

int MacroInstrument::GetTable() { return 0; };

bool MacroInstrument::GetTableAutomation() { /*return tableAuto_->GetBool();*/
  return false;
};

void MacroInstrument::GetTableState(TableSaveState &state){};

void MacroInstrument::SetTableState(TableSaveState &state){};
