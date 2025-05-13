#include "UIController.h"

#include "Application/Instruments/MidiInstrument.h"
#include "Application/Model/Project.h"
#include "Application/Player/Player.h"

UIController::UIController(){};

UIController *UIController::GetInstance() {
  if (instance_ == 0) {
    instance_ = new UIController();
  }
  return instance_;
}

void UIController::Init(Project *project, ViewData *viewData) {
  viewData_ = viewData;
  project_ = project;
}

void UIController::Reset() {
  viewData_ = 0;
  project_ = 0;
};

void UIController::UnMuteAll() {

  Player *player = Player::GetInstance();
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    player->SetChannelMute(i, false);
  };
};

void UIController::ToggleMute(int from, int to) {

  Player *player = Player::GetInstance();
  Project *project = player->GetProject();
  
  for (int i = from; i < to + 1; i++) {
    bool wasMuted = player->IsChannelMuted(i);
    bool willBeMuted = !wasMuted;
    
    // Check if there's a currently playing instrument on this channel
    // We'll only handle MIDI instruments that are currently playing
    I_Instrument *instrument = nullptr;
    if (player->IsChannelPlaying(i)) {
      // Get the instrument from the channel
      instrument = player->GetProject()->GetInstrumentBank()->GetInstrument(i % MAX_INSTRUMENT_COUNT);
    }
    
    // Set the mute state in the player
    player->SetChannelMute(i, willBeMuted);
    
    // If it's a MIDI instrument, handle the mute change
    if (instrument && instrument->GetType() == IT_MIDI) {
      MidiInstrument *midiInstrument = static_cast<MidiInstrument *>(instrument);
      midiInstrument->HandleMuteChange(i, willBeMuted);
    }
  };
};

void UIController::SwitchSoloMode(int from, int to, bool soloing) {

  Player *player = Player::GetInstance();
  Project *project = player->GetProject();

  // If not in solo mode, we solo current channel or selection
  if (soloing) {
    // Store current mute states and set new mute states
    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      soloMask_[i] = player->IsChannelMuted(i);
      bool willBeMuted = (i < from) || (i > to);
      
      // If the mute state is changing, handle it for MIDI instruments
      if (soloMask_[i] != willBeMuted) {
        // Set the mute state in the player
        player->SetChannelMute(i, willBeMuted);
        
        // Check if there's a currently playing instrument on this channel
        // We'll only handle MIDI instruments that are currently playing
        I_Instrument *instrument = nullptr;
        if (player->IsChannelPlaying(i)) {
          // Get the instrument from the channel
          instrument = player->GetProject()->GetInstrumentBank()->GetInstrument(i % MAX_INSTRUMENT_COUNT);
        }
            
        // If it's a MIDI instrument, handle the mute change
        if (instrument && instrument->GetType() == IT_MIDI) {
          MidiInstrument *midiInstrument = static_cast<MidiInstrument *>(instrument);
          midiInstrument->HandleMuteChange(i, willBeMuted);
        }
      } else {
        // No change in mute state, just set it
        player->SetChannelMute(i, willBeMuted);
      }
    };
  } else {
    // Restore previous mute states
    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      bool wasMuted = player->IsChannelMuted(i);
      bool willBeMuted = soloMask_[i];
      
      // If the mute state is changing, handle it for MIDI instruments
      if (wasMuted != willBeMuted) {
        // Set the mute state in the player
        player->SetChannelMute(i, willBeMuted);
        
        // Check if there's a currently playing instrument on this channel
        // We'll only handle MIDI instruments that are currently playing
        I_Instrument *instrument = nullptr;
        if (player->IsChannelPlaying(i)) {
          // Get the instrument from the channel
          instrument = player->GetProject()->GetInstrumentBank()->GetInstrument(i % MAX_INSTRUMENT_COUNT);
        }
            
        // If it's a MIDI instrument, handle the mute change
        if (instrument && instrument->GetType() == IT_MIDI) {
          MidiInstrument *midiInstrument = static_cast<MidiInstrument *>(instrument);
          midiInstrument->HandleMuteChange(i, willBeMuted);
        }
      } else {
        // No change in mute state, just set it
        player->SetChannelMute(i, willBeMuted);
      }
    };
  }
};
