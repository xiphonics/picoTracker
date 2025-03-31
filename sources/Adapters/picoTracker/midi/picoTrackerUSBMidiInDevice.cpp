#include "picoTrackerUSBMidiInDevice.h"
#include "Adapters/picoTracker/platform/platform.h"
#include "Services/Midi/MidiMessage.h"
#include "System/Console/Trace.h"
#include "pico/stdlib.h"
#include "tusb.h"

picoTrackerUSBMidiInDevice::picoTrackerUSBMidiInDevice(const char *name)
    : MidiInDevice(name) {
  Trace::Log("MIDI", "Created USB MIDI input device %s", name);
}

picoTrackerUSBMidiInDevice::~picoTrackerUSBMidiInDevice() { Close(); }

bool picoTrackerUSBMidiInDevice::Init() { // TinyUSB should already be
                                          // initialized in the main application
  Trace::Log("MIDI", "Initialized USB MIDI input driver");
  return true;
}

void picoTrackerUSBMidiInDevice::Close() {
  Trace::Log("MIDI", "Closed USB MIDI input driver");
}

bool picoTrackerUSBMidiInDevice::Start() {
  Trace::Log("MIDI", "Started USB MIDI input driver");
  return true;
}

void picoTrackerUSBMidiInDevice::Stop() {
  Trace::Log("MIDI", "Stopped USB MIDI input driver");
}

void picoTrackerUSBMidiInDevice::poll() {
  // Check if device is mounted and ready
  if (!tud_midi_mounted()) {
    return;
  }

  // MIDI packet format: [cable_number << 4 | code_index, midi_data1,
  // midi_data2, midi_data3]
  uint8_t packet[4];
  while (tud_midi_available()) {
    // Read the MIDI packet
    bool bytesRead = tud_midi_packet_read(packet);

    if (!bytesRead) {
      Trace::Error("Failed to read expected MIDI packet!");
      break;
    }
    // start at 1 to skip USB midi cable number
    for (unsigned short j = 1; j < bytesRead; j++) {
      // Trace::Debug("Processing byte: 0x%02X", data[j]);
      processMidiData(packet[j]);
    }
  }
}
