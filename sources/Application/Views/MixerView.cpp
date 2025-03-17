#include "MixerView.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "Application/Model/Mixer.h"
#include "Application/Utils/char.h"
#include "Application/Utils/mathutils.h"
#include "UIController.h"
#include <string>

#define CHANNELS_X_OFFSET_ 3 // stride between each channel

MixerView::MixerView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {
  invertBatt_ = false;
}

MixerView::~MixerView() {}

void MixerView::onStart() {
  Player *player = Player::GetInstance();
  unsigned char from = viewData_->songX_;
  unsigned char to = from;
  player->OnStartButton(PM_SONG, from, false, to);
};

void MixerView::onStop() {
  Player *player = Player::GetInstance();
  unsigned char from = viewData_->songX_;
  unsigned char to = from;
  player->OnStartButton(PM_SONG, from, true, to);
};

void MixerView::OnFocus(){};

void MixerView::updateCursor(int dx, int dy) {
  int x = viewData_->mixerCol_;
  x += dx;
  if (x < 0)
    x = 0;
  if (x > 7)
    x = 7;
  viewData_->mixerCol_ = x;
  isDirty_ = true;
}

void MixerView::switchSoloMode() {
  UIController *controller = UIController::GetInstance();
  int currentChannel = viewData_->mixerCol_;
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
  int currentChannel = viewData_->mixerCol_;
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
    return;
  };

  viewMode_ = VM_NORMAL;
  processNormalButtonMask(mask);
};

/******************************************************
 processNormalButtonMask:
        process button mask in the case there is no
        selection active
 ******************************************************/

void MixerView::processNormalButtonMask(unsigned int mask) {

  if (mask & EPBM_EDIT) {
    if (mask & EPBM_NAV) {
      toggleMute();
    };
  } else {
    if (mask & EPBM_ENTER) {
      if (mask & EPBM_NAV) {
        switchSoloMode();
      }
    } else {
      if (mask & EPBM_NAV) {
        if (mask & EPBM_UP) {
          ViewType vt = VT_SONG;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }
        if (mask & EPBM_PLAY) {
          onStop();
        }
        if (mask & EPBM_ALT) {
          unMuteAll();
        }
      } else {
        if (mask & EPBM_ALT) {

        } else {
          if (mask & EPBM_PLAY) {
            onStart();
          }
          if (mask & EPBM_LEFT)
            updateCursor(-1, 0);
          if (mask & EPBM_RIGHT)
            updateCursor(1, 0);
        }
      }
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

void MixerView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();
  GUIPoint anchor = GetAnchor();

  // Draw title
  SetColor(CD_NORMAL);

  Player *player = Player::GetInstance();

  props.invert_ = true;
  const char *buffer =
      ((player->GetSequencerMode() == SM_SONG) ? "Song" : "Live");
  DrawString(pos._x, pos._y, buffer, props);
  props.invert_ = false;

  // Now draw busses
  // we start at the bottom of the VU meter and draw it growing upwards
  pos = anchor;
  pos._y += VU_METER_HEIGHT - 1; // -1 to align with song grid
  props.invert_ = true;
  // get levels from the player
  etl::array<stereosample, 8> *levels = player->GetMixerLevels();
  drawChannelVUMeters(levels, player, props);

  SetColor(CD_NORMAL);
  props.invert_ = false;

  pos._y += 1;
  // draw bus states
  char state[3];
  state[0] = '-'; // M
  state[1] = '-';
  state[2] = '\0';
  for (int i = 0; i < 8; i++) {
    if (i == viewData_->mixerCol_) {
      props.invert_ = true;
      SetColor(CD_HILITE2);
    }
    state[0] = player->IsChannelMuted(i) ? 'M' : '-';

    DrawString(pos._x, pos._y, state, props);
    pos._x += CHANNELS_X_OFFSET_;
    if (i == viewData_->mixerCol_) {
      props.invert_ = false;
      SetColor(CD_NORMAL);
    }
  };

  drawMap();
  drawNotes();
  drawMasterVuMeter(player, props);

  if (player->IsRunning()) {
    OnPlayerUpdate(PET_UPDATE);
  };
};

void MixerView::OnPlayerUpdate(PlayerEventType eventType, unsigned int tick) {

  Player *player = Player::GetInstance();

  // Draw clipping indicator & CPU usage
  GUIPoint anchor = GetAnchor();
  GUIPoint pos = anchor;

  // get levels from the player
  etl::array<stereosample, 8> *levels = player->GetMixerLevels();

  GUITextProperties props;
  SetColor(CD_NORMAL);

  drawChannelVUMeters(levels, player, props);

  drawMasterVuMeter(player, props);

  pos = 0;
  pos._x = 27;
  pos._y += 1;
  if (eventType != PET_STOP) {
    drawPlayTime(player, pos, props);
  }

  drawNotes();
};

void MixerView::AnimationUpdate() {
  // redraw batt gauge on every clock tick (~1Hz) even when not playing
  // and not redrawing due to user cursor navigation
  GUITextProperties props;
  drawBattery(props);
  w_.Flush();
};

void MixerView::drawChannelVUMeters(etl::array<stereosample, 8> *levels,
                                    Player *player, GUITextProperties props) {

  // we start at the bottom of the VU meter and draw it growing upwards
  GUIPoint pos = GetAnchor();
  pos._y += VU_METER_HEIGHT - 1; // -1 to align with song grid

  // draw vu meter for each bus
  for (int i = 0; i < 8; i++) {
    int leftBars = 0;
    int rightBars = 0;
    // if channel is muted just use default 0 values for bars
    if (!player->IsChannelMuted(i)) {
      // Convert to dB
      int leftDb = amplitudeToDb((levels->at(i) >> 16) & 0xFFFF);
      int rightDb = amplitudeToDb(levels->at(i) & 0xFFFF);

      // Map dB to bar levels  -60dB to 0dB range mapped to 0-15 bars
      leftBars = std::max(0, std::min(VU_METER_HEIGHT, (leftDb + 60) / 4));
      rightBars = std::max(0, std::min(VU_METER_HEIGHT, (rightDb + 60) / 4));
    }

    drawVUMeter(leftBars, rightBars, pos, props);
    pos._x += CHANNELS_X_OFFSET_;
  }
}