---
title: MIDI Implementation
template: page
---

# MIDI Implementation

## MIDI Input

picoTracker supports MIDI input for real-time playback and control. This allows you to connect a MIDI keyboard or controller to play notes and control various parameters.

### Input Methods

picoTracker supports two MIDI input methods:

1. **USB MIDI**: Connect any USB MIDI device directly to the picoTracker's USB port. USB MIDI is supported on all picoTracker hardware versions.

2. **TRS MIDI (3.5mm)**: Connect standard MIDI devices using a MIDI to TRS adapter. **Note:** TRS MIDI input requires a v2.1 or newer picoTracker PCB _*_.

Both USB and TRS MIDI inputs are always enabled by default when the picoTracker starts up.

### MIDI Note Playback

When a MIDI device is connected and configured, incoming MIDI notes will trigger the corresponding instruments in picoTracker. Each MIDI channel is mapped to instrument slot in picoTracker. For now this mapping is fixed, eg. MIDI channel 1 will trigger instrument 00, MIDI channel 2 will trigger instrument 01, and so on.

#### Polyphonic Playback Limitations

**Important:** Not all instrument types support polyphonic MIDI input playback:

- **Sample Instruments**: Fully support polyphonic playback. Multiple notes can be played simultaneously, and each note can be stopped independently.

- **OPAL Instruments**: Should be considered monophonic from a MIDI input perspective. Playing a new note will stop any currently playing note on the same MIDI channel.

- **SID Instruments**: Should be considered monophonic from a MIDI input perspective. Playing a new note will stop any currently playing note on the same MIDI channel.

This limitation is due to how these instrument types handle note playback internally. Sample instruments maintain separate state for each audio channel, while OPAL and SID instruments use a single state that affects all notes played through them.

### Supported MIDI Messages

picoTracker currently supports the following MIDI message types:

- **Note On**: Triggers instrument playback. Each MIDI channel maps to a corresponding instrument.
- **Note Off**: Stops the corresponding note that was previously triggered by a Note On message.
- **Start**: Starts playback when receiving a MIDI Start message.
- **Stop**: Stops playback when receiving a MIDI Stop message.

The following message types are recognized but **not** implemented yet:

- **Clock**: Synchronizes picoTracker's tempo to an external MIDI clock source.
- **Aftertouch** (Polyphonic Pressure)
- **Control Change** (CC)
- **Program Change**
- **Channel Aftertouch** (Channel Pressure)
- **Pitch Bend**
- **Continue**

### MIDI Configuration

Currently, MIDI input is always enabled by default when picoTracker starts up. There is no specific configuration required to use MIDI input.

Each MIDI channel directly maps to an instrument index in picoTracker. For example, MIDI channel 1 will trigger instrument 1, MIDI channel 2 will trigger instrument 2, and so on.

## MIDI Output

*[MIDI output documentation to be added]*

 (v1.2 and v2.0 PCBs require a soldered hardware modification to enable TRS MIDI input)