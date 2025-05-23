#ifndef _MIDI_NOTE_TRACKER_H_
#define _MIDI_NOTE_TRACKER_H_

#include "Externals/etl/include/etl/array.h"
#include <cstdint>

// Maximum number of channels to track notes on
#define MAX_NOTE_CHANNELS 8

/**
 * MidiNoteTracker - Tracks active MIDI notes across multiple channels
 *
 * This class keeps track of which notes are currently active on which channels,
 * allowing for polyphonic note handling where a note is only stopped when
 * all instances of it have been released.
 */
class MidiNoteTracker {
public:
  MidiNoteTracker();
  ~MidiNoteTracker();

  /**
   * Register a note as active on a specific channel
   *
   * @param note The MIDI note number (0-127)
   * @param midiChannel The original MIDI channel (0-15)
   * @param instrumentChannel The instrument channel assigned (0-7)
   * @param velocity The note velocity (0-127)
   * @return True if the note was successfully registered
   */
  bool registerNote(uint8_t note, uint8_t midiChannel,
                    uint8_t instrumentChannel, uint8_t velocity);

  /**
   * Check if a note is currently active on any channel
   * 
   * @param note The MIDI note number (0-127)
   * @return True if the note is active on any channel
   */
  bool isNoteActive(uint8_t note) const;
  
  /**
   * Check if a specific note is active on a specific MIDI channel
   * 
   * @param note The MIDI note number (0-127)
   * @param midiChannel The MIDI channel (0-15)
   * @return True if the note is active on the specified channel
   */
  bool isNoteActiveOnChannel(uint8_t note, uint8_t midiChannel) const;
  
  /**
   * Get the audio channel assigned to a specific note on a specific MIDI channel
   * 
   * @param note The MIDI note number (0-127)
   * @param midiChannel The MIDI channel (0-15)
   * @return The audio channel assigned to the note, or -1 if not found
   */
  int getAudioChannelForNote(uint8_t note, uint8_t midiChannel) const;

  /**
   * Unregister a note on a specific MIDI channel
   *
   * @param note The MIDI note number (0-127)
   * @param midiChannel The MIDI channel (0-15)
   * @return The instrument channel that should be stopped, or -1 if the note
   * should not be stopped
   */
  int unregisterNote(uint8_t note, uint8_t midiChannel);

  /**
   * Get the next available instrument channel
   *
   * @return The next available instrument channel, or -1 if all channels are in
   * use
   */
  int getNextAvailableChannel() const;

  /**
   * Clear all tracked notes
   */
  void clear();

private:
  struct NoteInfo {
    bool active;
    uint8_t midiNote;          // The MIDI note number (0-127)
    uint8_t midiChannel;       // The original MIDI channel (0-15)
    uint8_t instrumentChannel; // The instrument channel assigned (0-7)
    uint8_t velocity;          // The note velocity (0-127)
  };

  // Array of 8 playing notes, one for each instrument channel
  etl::array<NoteInfo, MAX_NOTE_CHANNELS> playingNotes_;
};

#endif // _MIDI_NOTE_TRACKER_H_
