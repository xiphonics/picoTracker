---
title: MIDI Implementation
template: page
---

## MIDI Input

picoTracker supports MIDI input for real-time playback and control. This allows you to connect a MIDI keyboard or controller to play notes and control various parameters.

### Input Methods

picoTracker supports two MIDI input methods:

1. **USB MIDI**: Connect any USB MIDI device directly to the picoTracker's USB port. USB MIDI is supported on all picoTracker hardware versions.

2. **TRS MIDI (3.5mm)**: Connect standard MIDI devices using a MIDI to TRS adapter. **Note:** TRS MIDI input requires a v2.1 or newer picoTracker PCB _*_.

In addition to regular MIDI input, picoTracker Advance also allows for dedicated **MIDI control surfaces** that provide a hands-on live performance experience. See [MIDI Control Surfaces](midi-control-surfaces.html) for details on the compatible midi controllers.

### MIDI Note Playback

When a MIDI device is connected and configured, incoming MIDI notes are played through song channels assigned by `MIDI CHANNEL MAP` on the Project screen. A song channel with an assignment is reserved for MIDI input and is not played by the song sequencer.

While one or more incoming MIDI notes are playing, the top of the screen shows the play icon followed by `M`.

The Project screen's `MIDI DEFAULT PROGRAM` values select the instruments used after loading the project and before any Program Change is received. Each program from `00` to `3F` maps to each instrument. Changing the default program acts as a manual `PROGRAM CHANGE` command and will immediately set that new default program as the currently active for the channel. New projects use instrument `00` for all 16 MIDI channels. Some MIDI controllers label the Program Change value `00` as program 1, but picoTracker shows the instrument number it selects. Sending MIDI Program change commands higher than 64 will not select any instruments.

#### Polyphonic Playback
Assigning the same MIDI channel to several song channels creates a pool for polyphonic playing. Each held note uses one of those song channels until it is released. If every assigned song channel is occupied, additional notes are ignored until one becomes available.

### Supported MIDI Messages

picoTracker currently supports the following MIDI message types:

- **Note On**: Triggers instrument playback on a song channel assigned to the incoming MIDI channel.
- **Note Off**: Releases the corresponding note that was previously triggered by a Note On message. The instrument's release setting controls how long the note fades or holds afterward.
- **Program Change**: Selects the same-numbered instrument for subsequent notes on that MIDI channel.
- **Start**: Starts playback when receiving a MIDI Start message.
- **Stop**: Stops playback when receiving a MIDI Stop message.

The following message types are recognized but **not** implemented yet:

- **Clock**: Synchronizes picoTracker's tempo to an external MIDI clock source.
- **Aftertouch** (Polyphonic Pressure)
- **Control Change** (CC)
- **Channel Aftertouch** (Channel Pressure)
- **Pitch Bend**
- **Continue**

### MIDI Configuration

MIDI input setting on the Device screen needs to be enabled to use MIDI input.

Use `MIDI CHANNEL MAP` on the Project screen to choose which song channels receive each MIDI channel and `MIDI DEFAULT PROGRAM` to choose their initial instruments mapping. See [Project Management](projects.html) for details. The Song screen shows the resulting assignments in the `IN` row. This row is hidden when  no MIDI IN assignments exist.

## MIDI Output

MIDI output is supported via MIDI Instruments and related MIDI commands.
