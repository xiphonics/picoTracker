#include "MidiNoteTracker.h"
#include "System/Console/Trace.h"

MidiNoteTracker::MidiNoteTracker() {
  // Initialize all note tracking data
  clear();
}

MidiNoteTracker::~MidiNoteTracker() {
  // Nothing to clean up
}

bool MidiNoteTracker::registerNote(uint8_t note, uint8_t midiChannel,
                                   uint8_t instrumentChannel,
                                   uint8_t velocity) {
  // Validate parameters
  if (note > 127 || midiChannel > 15) {
    Trace::Debug("Invalid parameters in registerNote: note=%d, midiChannel=%d", 
                 note, midiChannel);
    return false;
  }

  // Find an available audio channel (one that's not active)
  int availableChannel = getNextAvailableChannel();

  // If no available channel, return false
  if (availableChannel == -1) {
    return false;
  }

  // Register the note in the available channel
  playingNotes_[availableChannel].active = true;
  playingNotes_[availableChannel].midiNote = note;
  playingNotes_[availableChannel].midiChannel = midiChannel;
  playingNotes_[availableChannel].instrumentChannel = availableChannel; // Audio channel = array index
  playingNotes_[availableChannel].velocity = velocity;
  
  Trace::Debug("Note %d registered on MIDI channel %d, audio channel %d",
               note, midiChannel, availableChannel);
  return true;
}

bool MidiNoteTracker::isNoteActive(uint8_t note) const {
  // Check if the note is active on any channel
  for (const auto &activeNote : playingNotes_) {
    if (activeNote.active && activeNote.midiNote == note) {
      return true;
    }
  }
  return false;
}

bool MidiNoteTracker::isNoteActiveOnChannel(uint8_t note,
                                            uint8_t midiChannel) const {
  // Check if the specific note is active on the specific MIDI channel
  for (const auto &activeNote : playingNotes_) {
    if (activeNote.active && activeNote.midiNote == note &&
        activeNote.midiChannel == midiChannel) {
      return true;
    }
  }
  return false;
}

int MidiNoteTracker::getAudioChannelForNote(uint8_t note,
                                            uint8_t midiChannel) const {
  // Find the audio channel assigned to this note on this MIDI channel
  for (size_t i = 0; i < playingNotes_.size(); i++) {
    const auto &activeNote = playingNotes_[i];
    if (activeNote.active && activeNote.midiNote == note &&
        activeNote.midiChannel == midiChannel) {
      // The audio channel is the index in the array
      return static_cast<int>(i);
    }
  }
  return -1; // Note not found
}

int MidiNoteTracker::getNextAvailableChannel() const {
  // Find an inactive channel
  for (size_t i = 0; i < playingNotes_.size(); i++) {
    if (!playingNotes_[i].active) {
      return static_cast<int>(i);
    }
  }

  // If all channels are active, return -1 (no available channels)
  return -1;
}

int MidiNoteTracker::unregisterNote(uint8_t note, uint8_t midiChannel) {
  // Find the note in the active notes list
  for (size_t i = 0; i < playingNotes_.size(); i++) {
    auto &activeNote = playingNotes_[i];
    if (activeNote.active && activeNote.midiNote == note &&
        activeNote.midiChannel == midiChannel) {
      // Found the note, mark it as inactive
      int audioChannel = static_cast<int>(i); // Audio channel = array index
      activeNote.active = false;
      Trace::Debug("Note %d unregistered from MIDI channel %d, stopping "
                   "audio channel %d",
                   note, midiChannel, audioChannel);
      return audioChannel;
    }
  }

  // Note not found
  Trace::Debug("Note %d not found on MIDI channel %d", note, midiChannel);
  return -1;
}

void MidiNoteTracker::clear() {
  // Mark all notes as inactive
  for (auto &note : playingNotes_) {
    note.active = false;
  }
}
