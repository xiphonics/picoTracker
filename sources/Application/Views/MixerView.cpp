/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "MixerView.h"
#include "Application/Model/Mixer.h"
#include "Application/Utils/char.h"
#include "Application/Utils/mathutils.h"
#include "Application/Views/SampleEditorView.h"
#include "UIController.h"
#include <Application/AppWindow.h>
#include <string>

#define CHANNELS_X_OFFSET_ 3 // stride between each channel

MixerView::MixerView(GUIWindow &w, ViewData *viewData)
    : FieldView(w, viewData) {

  // Initialize the channel volume fields
  initChannelVolumeFields();
}

MixerView::~MixerView() {}

void MixerView::OnFocus() {
  // update selected field to match current cursor position
  if (viewData_->songX_ <= SONG_CHANNEL_COUNT) {
    if (viewData_->songX_ < SONG_CHANNEL_COUNT) {
      // Channel 0-7
      SetFocus((UIField *)&channelVolumeFields_.at(viewData_->songX_));
    } else {
      // Master channel
      SetFocus((UIField *)&masterVolumeField_.at(0));
    }
  }
};

void MixerView::SetFocus(UIField *field) {
  // Call parent implementation first
  FieldView::SetFocus(field);

  // Now update songX_ based on which field has focus
  if (!field)
    return;

  // Check if it's one of the channel volume fields
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    if (field == (UIField *)&channelVolumeFields_.at(i)) {
      viewData_->songX_ = i;
      return;
    }
  }

  // Check if it's the master volume field
  if (field == (UIField *)&masterVolumeField_.at(0)) {
    viewData_->songX_ = SONG_CHANNEL_COUNT;
  }
}

// keep track of currently selected channel
void MixerView::updateCursor(int dx, int dy) {
  int x = viewData_->songX_;
  x += dx;

  // Prevent wrapping by clamping values
  if (x < 0) {
    x = 0;
  }
  if (x > SONG_CHANNEL_COUNT) {
    x = SONG_CHANNEL_COUNT;
  }
  viewData_->songX_ = x;

  // Update field focus to match the selected channel
  if (x < SONG_CHANNEL_COUNT) {
    // Channel 0-7
    SetFocus(&channelVolumeFields_[x]);
  } else {
    // Master channel
    SetFocus(&masterVolumeField_[0]);
  }

  isDirty_ = true;
}

void MixerView::switchSoloMode() {
  UIController *controller = UIController::GetInstance();
  int currentChannel = viewData_->songX_;
  controller->SwitchSoloMode(currentChannel, currentChannel,
                             (viewMode_ == VM_NORMAL));
  viewMode_ = (viewMode_ != VM_SOLOON) ? VM_SOLOON : VM_NORMAL;
  isDirty_ = true;
};

void MixerView::unMuteAll() {
  UIController *controller = UIController::GetInstance();
  controller->UnMuteAll();
  isDirty_ = true;
};

void MixerView::toggleMute() {

  UIController *controller = UIController::GetInstance();
  int currentChannel = viewData_->songX_;
  controller->ToggleMute(currentChannel, currentChannel);
  viewMode_ = (viewMode_ != VM_MUTEON) ? VM_MUTEON : VM_NORMAL;
  isDirty_ = true;
};

void MixerView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed) {
    if (viewMode_ == VM_MUTEON) {
      if (mask & EPBM_NAV) {
        toggleMute();
      }
    };
    if (viewMode_ == VM_SOLOON) {
      if (mask & EPBM_NAV) {
        switchSoloMode();
      }
    };
    // Force a full redraw of the mixer view
    SetDirty(true);
    return;
  };

  // First check if we need to handle special mixer-specific actions
  if (!pressed) {
    if (viewMode_ == VM_MUTEON) {
      if (mask & EPBM_NAV) {
        toggleMute();
      }
    };
    if (viewMode_ == VM_SOLOON) {
      if (mask & EPBM_NAV) {
        switchSoloMode();
      }
    };
    return;
  };

  // Ignore up/down arrow keys when pressed by themselves in MixerView
  // We only want left/right to navigate between channels
  if (mask == EPBM_UP || mask == EPBM_DOWN) {
    return;
  }

  // Fieldview gets first go at the button event
  FieldView::ProcessButtonMask(mask, pressed);

  // Handle playback specific actions
  if (mask == EPBM_PLAY) {
    togglePlay();
    return;
  }
  // NAV back to Song view
  if ((mask & EPBM_NAV) && (mask & EPBM_UP)) {
    ViewType vt = VT_SONG;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
    return;
  }
  if ((mask & EPBM_NAV) && (mask & EPBM_ALT)) {
    unMuteAll();
    return;
  }

  // Handle mixer-specific actions for normal mode, but only if we don't have
  // field focus or if they're specific mixer actions that should override field
  // editing

  // EDIT+NAV is always for toggling mute
  if ((mask & EPBM_EDIT) && (mask & EPBM_NAV)) {
    toggleMute();
    return;
  }

  // ENTER+NAV is always for solo mode
  if ((mask & EPBM_ENTER) && (mask & EPBM_NAV)) {
    switchSoloMode();
    return;
  }

  viewMode_ = VM_NORMAL;
  // Force a full redraw of the mixer view when any button is pressed
  SetDirty(true);
  processNormalButtonMask(mask);
};

/******************************************************
 processNormalButtonMask:
        process button mask in the case there is no
        selection active
 ******************************************************/

// This method is no longer needed as we're using FieldView's field navigation
void MixerView::processNormalButtonMask(unsigned int mask) {
  // Handle mixer-specific actions
  if (mask & EPBM_EDIT) {
    if (mask & EPBM_NAV) {
      toggleMute();
    }
    if (mask & EPBM_PLAY) {
      // recording screen
      if (!Player::instance().IsRunning()) {
        SampleEditorView::SetSourceViewType(VT_MIXER);
        ViewType vt = VT_RECORD;
        ViewEvent ve(VET_SWITCH_VIEW, &vt);
        SetChanged();
        NotifyObservers(&ve);
      }
    }
  } else if (mask & EPBM_ENTER) {
    if (mask & EPBM_NAV) {
      switchSoloMode();
    }
  } else if (mask & EPBM_NAV) {
    if (mask & EPBM_UP) {
      ViewType vt = VT_SONG;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
    if (mask & EPBM_ALT) {
      unMuteAll();
    }
  } else {
    if (mask & EPBM_LEFT) {
      updateCursor(-1, 0);
    }
    if (mask & EPBM_RIGHT) {
      updateCursor(1, 0);
    }
  }
};

/******************************************************
 processSelectionButtonMask:
        process button mask in the case there is a
        selection active
 ******************************************************/
void MixerView::processSelectionButtonMask(unsigned int mask) {

  if (mask & EPBM_EDIT) {

  } else {
    if (mask & EPBM_ENTER) {
      if (mask & EPBM_NAV) {
        switchSoloMode();
      }
    } else {
      if (mask & EPBM_NAV) {
        if (mask & EPBM_PLAY) {
          onStop();
        }
        if (mask & EPBM_ALT) {
          unMuteAll();
        }
      } else {
        // No modifier
        if (mask & EPBM_PLAY) {
          onStart();
        }
      }
    }
  }
}

void MixerView::initChannelVolumeFields() {
  // Get the project from the Player
  Project *project =
      Player::is_valid() ? Player::instance().GetProject() : nullptr;

  if (!project)
    return;

  // Position for volume fields - below VU meters
  GUIPoint position = GetAnchor();
  position._y += VU_METER_HEIGHT + 1; // Position below VU meters

  // Get FourCC codes for channel volumes
  FourCC channelVolumeFourCCs[SONG_CHANNEL_COUNT] = {
      FourCC::VarChannel1Volume, FourCC::VarChannel2Volume,
      FourCC::VarChannel3Volume, FourCC::VarChannel4Volume,
      FourCC::VarChannel5Volume, FourCC::VarChannel6Volume,
      FourCC::VarChannel7Volume, FourCC::VarChannel8Volume};

  // Clear any existing fields
  channelVolumeFields_.clear();

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    // Create position for this channel's volume field
    GUIPoint fieldPos = position;
    fieldPos._x = position._x + (i * CHANNELS_X_OFFSET_);

    // Find the variable for this channel's volume
    Variable *v = project->FindVariable(channelVolumeFourCCs[i]);
    if (v) {
      // Create a 2-digit field (00-99) for the channel volume
      // Format: %2.2d = 2-digit decimal number with leading zeros
      // Use xOffset=1 and yOffset=5 for small/large increments
      channelVolumeFields_.emplace_back(fieldPos, *v, "%2.2d", 0, 99, 1, 5);

      // Add the field to the fieldList_ for proper field navigation
      fieldList_.insert(fieldList_.end(), &(*channelVolumeFields_.rbegin()));
    }
  }

  // Add master volume field to the right of channel volumes
  GUIPoint masterPos = position;
  // Position to the right of channel volumes
  masterPos._x += (SONG_CHANNEL_COUNT * CHANNELS_X_OFFSET_);

  Variable *v = project->FindVariable(FourCC::VarMasterVolume);
  if (v) {
    masterVolumeField_.emplace_back(masterPos, *v, "%2.2d", 0, 100, 1, 5);
    fieldList_.insert(fieldList_.end(), &(*masterVolumeField_.begin()));
  }

  // Set focus to the first field if we have any fields
  if (!fieldList_.empty()) {
    SetFocus(*fieldList_.begin());
  }
}

void MixerView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();
  GUIPoint anchor = GetAnchor();

  // Draw title
  SetColor(CD_NORMAL);

  Project *project =
      Player::instance().GetProject(); // Use Player's GetProject method

  props.invert_ = true;
  const char *buffer =
      ((Player::instance().GetSequencerMode() == SM_SONG) ? "Song" : "Live");
  DrawString(pos._x, pos._y, buffer, props);
  props.invert_ = false;

  // Now draw busses
  // we start at the bottom of the VU meter and draw it growing upwards
  pos = anchor;
  pos._y += VU_METER_HEIGHT - 1; // -1 to align with song grid
  props.invert_ = true;

  // get levels from the player
  // etl::array<stereosample, SONG_CHANNEL_COUNT> *levels =
  //     player->GetMixerLevels();
  // drawChannelVUMeters(levels, player, props);

  SetColor(CD_NORMAL);
  props.invert_ = false;

  // Draw all fields (channel volume fields)
  FieldView::Redraw();

  // Draw mute indicators below the volume values
  pos._y = anchor._y + VU_METER_HEIGHT + 2; // Position below volume fields
  pos._x = GetTitlePosition()._x;

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    if (i == viewData_->songX_) {
      props.invert_ = true;
      SetColor(CD_HILITE2);
    }

    char state[2];
    state[0] = Player::instance().IsChannelMuted(i) ? 'M' : '-';
    state[1] = '\0';

    DrawString(pos._x, pos._y, state, props);
    pos._x += CHANNELS_X_OFFSET_;

    if (i == viewData_->songX_) {
      props.invert_ = false;
      SetColor(CD_NORMAL);
    }
  }

  drawMap();
  drawNotes();
  drawMasterVuMeter(props);

  // Draw master volume label
  GUIPoint labelPos = GetAnchor();
  // Align with master volume control
  labelPos._x += (SONG_CHANNEL_COUNT * CHANNELS_X_OFFSET_);
  labelPos._y = SCREEN_HEIGHT - 3; // Position below the volume control
  SetColor(CD_HILITE2);
  DrawString(labelPos._x, labelPos._y, "MB", props);
  SetColor(CD_NORMAL);

  if (Player::instance().IsRunning()) {
    OnPlayerUpdate(PET_UPDATE);
  };
};

void MixerView::OnPlayerUpdate(PlayerEventType eventType, unsigned int tick) {
  // Since this can be called from core1 via the Observer pattern,
  // we need to ensure we don't try directly calling draw functions here!

  // Instead of drawing directly, we'll just update our state and let
  // AnimationUpdate handle the actual drawing

  if (eventType != PET_STOP) {
    // Flag that play time needs to be updated
    needsPlayTimeUpdate_ = true;
  }

  needsNotesUpdate_ = true;
};

void MixerView::AnimationUpdate() {
  // First call the parent class implementation to draw the battery gauge
  ScreenView::AnimationUpdate();
  GUITextProperties props;

  // Only process updates below if we're fully initialized
  if (!viewData_ || !Player::is_valid()) {
    // Just flush the battery gauge and return
    w_.Flush();
    return;
  }

  // Always update VU meters, whether the sequencer is running or not
  // This ensures we see VU meter updates from MIDI input even when not playing
  etl::array<stereosample, SONG_CHANNEL_COUNT> *levels =
      Player::instance().GetMixerLevels();
  if (levels) {
    drawChannelVUMeters(levels, props);
    drawMasterVuMeter(props);
  }

  // Handle any pending updates from OnPlayerUpdate
  // This ensures all UI drawing happens in the same thread (core0)
  if (needsPlayTimeUpdate_) {
    GUIPoint pos = GetAnchor();
    // explicitly position timer directly below the battery gauge
    pos._x = 27;
    pos._y = 1;
    drawPlayTime(pos, props);
    needsPlayTimeUpdate_ = false;
  }

  if (needsNotesUpdate_) {
    drawNotes();
    needsNotesUpdate_ = false;
  }

  // Flush the window to ensure changes are displayed
  w_.Flush();
};

void MixerView::drawChannelVUMeters(
    etl::array<stereosample, SONG_CHANNEL_COUNT> *levels,
    GUITextProperties props, bool forceRedraw) {

  // Quick optimization: If not forcing redraw, check if any levels have changed
  // This saves CPU cycles by avoiding unnecessary drawing operations
  if (!forceRedraw) {
    bool anyChanges = false;
    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      // Convert to dB
      int leftDb = amplitudeToDb((levels->at(i) >> 16) & 0xFFFF);
      int rightDb = amplitudeToDb(levels->at(i) & 0xFFFF);

      // Map dB to bar levels
      int leftBars = std::max(0, std::min(VU_METER_HEIGHT, (leftDb + 60) / 4));
      int rightBars =
          std::max(0, std::min(VU_METER_HEIGHT, (rightDb + 60) / 4));

      // Check if this channel's levels have changed
      if (leftBars != prevLeftVU_[i + 1] || rightBars != prevRightVU_[i + 1]) {
        anyChanges = true;
        break;
      }
    }

    // If no changes, return early
    if (!anyChanges) {
      return;
    }
  }

  // we start at the bottom of the VU meter and draw it growing upwards
  GUIPoint pos = GetAnchor();
  pos._y += VU_METER_HEIGHT - 1; // -1 to align with song grid

  // draw vu meter for each bus
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    int leftBars = 0;
    int rightBars = 0;
    // if channel is muted just use default 0 values for bars
    if (!Player::instance().IsChannelMuted(i)) {
      // Convert to dB
      int leftDb = amplitudeToDb((levels->at(i) >> 16) & 0xFFFF);
      int rightDb = amplitudeToDb(levels->at(i) & 0xFFFF);

      // Map dB to bar levels  -60dB to 0dB range mapped to 0-15 bars
      leftBars = std::max(0, std::min(VU_METER_HEIGHT, (leftDb + 60) / 4));
      rightBars = std::max(0, std::min(VU_METER_HEIGHT, (rightDb + 60) / 4));
    }

    // Use index i+1 for channel VU meters (index 0 is reserved for master)
    drawVUMeter(leftBars, rightBars, pos, props, i + 1, forceRedraw);
    pos._x += CHANNELS_X_OFFSET_;
  }
}

void MixerView::togglePlay() {
  Player::instance().OnStartButton(PM_CHAIN, viewData_->songX_, true,
                                   viewData_->chainRow_);
};
