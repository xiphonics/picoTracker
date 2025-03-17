#include "MixerView.h"
#include "Application/Model/Mixer.h"
#include "Application/Utils/char.h"
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

void MixerView::ProcessButtonMask(unsigned short mask, bool pressed) {
  // if (!pressed) {
  //	if (viewMode_==VM_MUTEON) {
  //		if (mask&EPBM_R) {
  //			toggleMute() ;
  //		}
  //	} ;
  //	if (viewMode_==VM_SOLOON) {
  //		if (mask&EPBM_R) {
  //			switchSoloMode() ;
  //		}
  //	} ;
  //	return ;
  // } ;
  //

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
  } else {
    if (mask & EPBM_ENTER) {
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
    } else {
      if (mask & EPBM_NAV) {
        if (mask & EPBM_PLAY) {
          onStop();
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
  drawChannelVUMeters(levels, props);

  SetColor(CD_NORMAL);
  props.invert_ = false;

  pos._y += 1;
  // draw bus states
  char state[3];
  state[0] = 'M';
  state[1] = 'S';
  state[2] = '\0';
  for (int i = 0; i < 8; i++) {
    if (i == viewData_->mixerCol_) {
      props.invert_ = true;
      SetColor(CD_HILITE2);
    }

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

  drawChannelVUMeters(levels, props);

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
                                    GUITextProperties props) {

  // we start at the bottom of the VU meter and draw it growing upwards
  GUIPoint pos = GetAnchor();
  pos._y += VU_METER_HEIGHT - 1; // -1 to align with song grid

  // draw vu meter for each bus
  for (int i = 0; i < 8; i++) {
    // top 16bits is left channel, bottom 16bits is right
    // TODO: fix to use dB not linear
    // for now for linear divide by 4096
    unsigned char levelL = (levels->at(i) >> 16) / 4096;
    unsigned char levelR = (levels->at(i) & 0x0000FFFF) / 4096;

    // Trace::Debug("VUs[%d] %d %d", i, levelL, levelR);
    if (levelL > 15 || levelR > 15) {
      continue;
    }

    drawVUMeter(levelL, levelR, pos, props);
    pos._x += CHANNELS_X_OFFSET_;
  }
}