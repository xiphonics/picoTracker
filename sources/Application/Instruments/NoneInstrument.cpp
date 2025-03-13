#include "NoneInstrument.h"
#include "Externals/etl/include/etl/string.h"

NoneInstrument::NoneInstrument() : I_Instrument(&variables_) {}

NoneInstrument::~NoneInstrument(){};

bool NoneInstrument::Init() { return true; };

void NoneInstrument::OnStart(){};

bool NoneInstrument::Start(int c, unsigned char note, bool retrigger) {
  return true;
};

void NoneInstrument::Stop(int c){};

stereosample NoneInstrument::Render(int channel, fixed *buffer, int size,
                                    bool updateTick) {
  return 0;
};

bool NoneInstrument::IsInitialized() {
  return true; // Always initialised
};

void NoneInstrument::ProcessCommand(int channel, FourCC cc, ushort value){};

etl::string<24> NoneInstrument::GetName() { return name_; }

int NoneInstrument::GetTable() { return 0; };

bool NoneInstrument::GetTableAutomation() { return false; };

void NoneInstrument::GetTableState(TableSaveState &state){};

void NoneInstrument::SetTableState(TableSaveState &state){};
