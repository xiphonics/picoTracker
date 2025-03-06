#include "MixerView.h"
#include "Application/Model/Mixer.h"
#include "Application/Utils/char.h"
#include <string>

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
  const u_int8_t dx = 3;

  // get levels from the player
  auto levels = player->GetMixerLevels();

  // draw vu meter for each bus
  for (int j = 0; j < VU_METER_HEIGHT; j++) {
    for (int i = 0; i < 8; i++) {
      if (j == VU_METER_CLIP_LEVEL) {
        SetColor(CD_ERROR);
      } else if (j > VU_METER_WARN_LEVEL) {
        SetColor(CD_WARN);
      } else {
        SetColor(CD_INFO);
      }
      auto level = levels[i];
      Trace::Debug("lvl:%d", fp2i(level));
      if (level / 1024 < j) {
        DrawString(pos._x + (i * dx), pos._y - j, " ", props);
      }
      SetColor(CD_HILITE1);
      DrawString(pos._x + (i * dx) + 1, pos._y - j, " ", props);
    }
  };
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
    pos._x += dx;
    if (i == viewData_->mixerCol_) {
      props.invert_ = false;
      SetColor(CD_NORMAL);
    }
  };

  drawMap();
  drawNotes();
  drawMasterVuMeter(player, pos, props);

  if (player->IsRunning()) {
    OnPlayerUpdate(PET_UPDATE);
  };
};

void MixerView::OnPlayerUpdate(PlayerEventType type, unsigned int tick) {

  Player *player = Player::GetInstance();

  // Draw clipping indicator & CPU usage

  GUIPoint anchor = GetAnchor();
  GUIPoint pos = anchor;

  GUITextProperties props;
  SetColor(CD_NORMAL);

  if (player->Clipped()) {
    DrawString(pos._x, pos._y, "clip", props);
  } else {
    DrawString(pos._x, pos._y, "----", props);
  }
  char strbuffer[12];

  pos._y += 1;
  sprintf(strbuffer, "P:%3.3d%%", player->GetPlayedBufferPercentage());
  DrawString(pos._x, pos._y, strbuffer, props);

  System *sys = System::GetInstance();
  int batt = sys->GetBatteryLevel();
  if (batt >= 0) {
    if (batt < 90) {
      SetColor(CD_HILITE2);
      invertBatt_ = !invertBatt_;
    } else {
      invertBatt_ = false;
    };
    props.invert_ = invertBatt_;

    pos._y += 1;
    sprintf(strbuffer, "%B:3.3d%", batt);
    DrawString(pos._x, pos._y, strbuffer, props);
  }
  SetColor(CD_NORMAL);
  props.invert_ = false;
  int time = int(player->GetPlayTime());
  int mi = time / 60;
  int se = time - mi * 60;
  sprintf(strbuffer, "%2.2d:%2.2d", mi, se);
  pos._y += 1;
  DrawString(pos._x, pos._y, strbuffer, props);

  drawNotes();
};

void MixerView::AnimationUpdate() {
  // redraw batt gauge on every clock tick (~1Hz) even when not playing
  // and not redrawing due to user cursor navigation
  GUITextProperties props;
  drawBattery(props);
  w_.Flush();
};