/* SPDX-License-Identifier: BSD-3-Clause */
#include "Application/Instruments/I_Instrument.h"
#include "Services/Midi/MidiService.h"
#define NANOPRINTF_IMPLEMENTATION
#include "System/Console/nanoprintf.h"
#include <vector>

std::vector<MidiMessage> queuedMidiMessages;

MidiService::MidiService() = default;
MidiService::~MidiService() = default;
void MidiService::Update(Observable &, I_ObservableData *) {}
void MidiService::QueueMessage(MidiMessage &message) {
  queuedMidiMessages.push_back(message);
}
void MidiService::RegisterActiveChannel(uint8_t) {}

// Persistence and table playback are outside the instrument test scope.
SubService::SubService(int32_t fourCC, bool registerWithService)
    : fourCC_(fourCC), registerWithService_(registerWithService) {}
SubService::~SubService() = default;
Persistent::Persistent(const char *nodeName, bool registerWithService)
    : SubService(FourCC::ServicePersistency, registerWithService),
      nodeName_(nodeName) {}
I_Instrument::~I_Instrument() = default;
void I_Instrument::SaveContent(tinyxml2::XMLPrinter *) {}
void I_Instrument::RestoreContent(PersistencyDocument *) {}
void I_Instrument::Purge() {}
void TableSaveState::Reset() {}

#include "Services/Audio/AudioMixer.h"
void AudioMixer::AddModule(AudioModule &) {}
void AudioMixer::RemoveModule(AudioModule &) {}
