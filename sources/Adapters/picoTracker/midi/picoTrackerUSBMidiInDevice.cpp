/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

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

  // USB MIDI packet format: [cable_number << 4 | code_index, midi_data1,
  // midi_data2, midi_data3]
  uint8_t packet[4];

  while (tud_midi_available()) {
    // Read the MIDI packet
    uint32_t bytesRead = tud_midi_packet_read(packet);

    if (bytesRead == 0) {
      Trace::Error("Failed to read expected MIDI packet!");
      break;
    }

    // Extract the Code Index Number (CIN) from the first byte
    uint8_t cin = packet[0] & 0x0F;

    // Process the packet based on the CIN
    // The Code Index Number (CIN) indicates the classification of the bytes in
    // the MIDI_x fields
    // See Table 4-1 in the USB MIDI 1.0 specification PDF
    // The bytes in positions 1, 2, and 3 are the actual MIDI message
    // We just need to feed them to the MIDI parser

    // Only process valid MIDI messages (CIN values 0x2 through 0xF)
    if (cin >= 0x2) {
      // Feed each byte of the MIDI message to the parser
      // Start with the status byte
      processMidiData(packet[1]);

      // For CIN values 0x2-0xE, we need to process additional data bytes
      if (cin <= 0xE) {
        // Add data byte 1 for all message types
        processMidiData(packet[2]);

        // Add data byte 2 for 3-byte messages (CIN 0x8-0xB, 0xE)
        if ((cin >= 0x8 && cin <= 0xB) || cin == 0xE) {
          processMidiData(packet[3]);
        }
      }
    }
  }
}
