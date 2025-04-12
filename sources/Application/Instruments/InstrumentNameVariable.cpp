#include "InstrumentNameVariable.h"

InstrumentNameVariable::InstrumentNameVariable(I_Instrument *instrument)
    : Variable(FourCC::InstrumentName, instrument->GetDisplayName().c_str()),
      instrument_(instrument) {
  // Initialize with the instrument's current display name
}

InstrumentNameVariable::~InstrumentNameVariable() {}

etl::string<MAX_VARIABLE_STRING_LENGTH> InstrumentNameVariable::GetString() {
  // Get the name directly from the instrument
  return instrument_->GetUserSetName();
}

void InstrumentNameVariable::SetString(const char *string, bool notify) {
  // Set the name directly on the instrument
  instrument_->SetName(string);

  // Notify observers if requested
  if (notify) {
    SetChanged();
    NotifyObservers();
  }
}
