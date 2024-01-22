#include "MacroInstrument.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "Application/Instruments/Filters.h"
#include "Application/Model/Table.h"
#include "Application/Player/PlayerMixer.h" // For MIX_BUFFER_SIZE.. kick out pls
#include "Application/Player/SyncMaster.h"
#include "CommandList.h"
#include "Services/Audio/Audio.h"
#include "System/Console/Trace.h"
#include "System/io/Status.h"
#include "pico/rand.h"
#include <assert.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

uint8_t MacroInstrument::sync_samples_[4096];

MacroInstrument::MacroInstrument() {
  // Initialize exported variables
  //  auto setting_ = braids::SETTING_OSCILLATOR_SHAPE;
  //   uint8_t value = braids::settings.GetValue(setting_);
  /*  if (setting_ == braids::SETTING_OSCILLATOR_SHAPE &&
      braids::settings.meta_modulation()) {
    value = meta_shape_;
    }*/
  WatchedVariable *wv =
      new WatchedVariable("shape", BIP_SHAPE, braids::algo_values,
                          braids::MACRO_OSC_SHAPE_LAST - 2, 0);
  Insert(wv);
  wv->AddObserver(*this);

  //  Variable *v = new Variable("freq", BIP_FREQ, 0x7F);
  //  Insert(v);

  Variable *v = new Variable("timbre", BIP_TIMBRE, 0x7F);
  Insert(v);

  v = new Variable("color", BIP_COLOR, 0x7F);
  Insert(v);
}

MacroInstrument::~MacroInstrument() {}

bool MacroInstrument::Init() {
  Variable *vShape = FindVariable(BIP_SHAPE);
  shape_ = static_cast<braids::MacroOscillatorShape>(vShape->GetInt());

  osc_.Init();
  quantizer_.Init();
  envelope_.Init();
  ws_.Init(get_rand_32());
  jitter_source_.Init();

  return true;
}

void MacroInstrument::OnStart(){/* tableState_.Reset();*/};

bool MacroInstrument::Start(int channel, unsigned char midinote,
                            bool cleanstart) {
  envelope_.Update(0, 5 * 8); // Attack and Decay
  //  uint32_t ad_value = envelope_.Render();
  osc_.set_shape(shape_);
  osc_.set_parameters(0, 0); // timbre and color

  // TODO: Do pitch right, what is expected by braids?
  osc_.set_pitch(int16_t(440.0 * pow(2.0, (midinote - 69.0) / 12.0)));
  printf("pitch is %i\n", int16_t(440.0 * pow(2.0, (midinote - 69.0) / 12.0)));
  printf("midinote is %i\n", midinote);
  return true;
}

void MacroInstrument::Stop(int channel) { /*running_ = false; */
}

// Size in samples
bool MacroInstrument::Render(int channel, fixed *buffer, int size,
                             bool updateTick) {
  //  int start = micros();

  //    uint8_t *sync_buffer = sync_samples[render_block];
  //  int16_t *render_buffer = audio_samples[render_block];
  uint8_t *sync_buffer = sync_samples_;
  int16_t *render_buffer = (int16_t *)buffer;

  osc_.Render(sync_buffer, render_buffer, size);

  /*
  int16_t held_sample = 0;
  uint16_t signature = 0;
  size_t decimation_factor = 4;
  int32_t gain = 65535;
  uint16_t bit_mask = 0xff00;
  for (int i = 0; i < size; i++) {
    if ((i % decimation_factor) == 0) {
      held_sample = buffer[i] & bit_mask;
    }
    int16_t sample = held_sample * gain_lp_ >> 16;
    gain_lp_ += (gain - gain_lp_) >> 4;
    int16_t warped = ws_.Transform(sample);
    buffer[i] = braids::Mix(sample, warped, signature);
    //    printf("Buffer: %i\n)", buffer[i]);
    }*/

  //  printf("Render time: %i\n", micros() - start);
  return true;
};

bool MacroInstrument::IsInitialized() { /*return (source_ != 0); */
  return true;
};

void MacroInstrument::Update(Observable &o, I_ObservableData *d){};

void MacroInstrument::ProcessCommand(int channel, FourCC cc, ushort value){};

const char *MacroInstrument::GetName() {
  Variable *v = FindVariable(BIP_SHAPE);
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
