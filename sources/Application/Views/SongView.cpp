#include "SongView.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "UIController.h"
#include "ViewData.h"
#include <stdlib.h>
#include <string>

/****************
 Constructor
 ****************/

SongView::SongView(GUIWindow &w, ViewData *viewData, const char *song)
    : View(w, viewData) {

  updatingChain_ = false;
  lastChain_ = 0;
  songname_ = song;

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    this->lastPlayedPosition_[i] = 0;
    this->lastQueuedPosition_[i] = 0;
  }
  clipboard_.active_ = false;
  clipboard_.data_ = 0;
}

/****************
 Destructor
 ****************/

SongView::~SongView() {
  if (clipboard_.data_ != 0)
    SYS_FREE((void *)clipboard_.data_);
};

/******************************************************
 updateChain:
        update current chain value by adding offset
        parameter
 ******************************************************/

void SongView::updateChain(int offset) {

  unsigned int chain = viewData_->UpdateSongChain(offset);
  updatingChain_ = true;
  lastChain_ = chain;
  updateX_ = viewData_->songX_;
  updateY_ = viewData_->songY_;
  isDirty_ = true;
}

/******************************************************
 updateChain:
        set current chain value to value parameter
 ******************************************************/

void SongView::setChain(unsigned char value) {
  viewData_->SetSongChain(value);
  lastChain_ = value;
  isDirty_ = true;
}

/******************************************************
 updateSongOffset:
        modify top of the page row in song view by
        adding offset parameter
 ******************************************************/

void SongView::updateSongOffset(int offset) {
  viewData_->UpdateSongOffset(offset);
  isDirty_ = true;
}

/******************************************************
 updateCursor:
        modify location of cursor in view by
        adding dx & dy parameters
 ******************************************************/

void SongView::updateCursor(int dx, int dy) {
  viewData_->UpdateSongCursor(dx, dy);
  isDirty_ = true;
}

/******************************************************
 cutPosition:
        copy current position content to clipboard &
        erase current position value
 ******************************************************/

void SongView::cutPosition() {

  // prepare selection data
  clipboard_.x_ = viewData_->songX_;
  clipboard_.y_ = viewData_->songY_;
  clipboard_.offset_ = viewData_->songOffset_;

  saveX_ = viewData_->songX_;
  saveY_ = viewData_->songY_;
  saveOffset_ = viewData_->songOffset_;

  // cut selection
  cutSelection();
};

/******************************************************
 pastePosition:
        set current position to last chain value if
        current step is empty
 ******************************************************/

void SongView::pasteLast() {

  // If we're on an empty spot, we past the last chain
  // otherwise we take the current chain as last

  unsigned char *c = viewData_->GetCurrentSongPointer();
  if (*c == 0xFF) {
    *c = lastChain_;
    viewData_->song_->chain_.SetUsed(*c);
    isDirty_ = true;
  } else {
    lastChain_ = *c;
  }
};

/******************************************************
 clonePosition:
        slim clone current position
 ******************************************************/

void SongView::clonePosition() {

  unsigned char *pos = viewData_->GetCurrentSongPointer();
  unsigned char current = *pos;
  if (current == 255)
    return;

  unsigned short next = viewData_->song_->chain_.GetNext();
  if (next == NO_MORE_CHAIN)
    return;

  unsigned char *src = viewData_->song_->chain_.data_ + 16 * current;
  unsigned char *dst = viewData_->song_->chain_.data_ + 16 * next;

  for (int i = 0; i < 16; i++) {
    *dst++ = *src++;
  };

  src = viewData_->song_->chain_.transpose_ + 16 * current;
  dst = viewData_->song_->chain_.transpose_ + 16 * next;

  for (int i = 0; i < 16; i++) {
    *dst++ = *src++;
  };
  setChain((unsigned char)next);
  isDirty_ = true;
};

void SongView::extendSelection() {
  GUIRect rect = getSelectionRect();
  if (rect.Left() > 0 || rect.Right() < 7) {
    if (viewData_->songX_ < clipboard_.x_) {
      viewData_->songX_ = 0;
      clipboard_.x_ = 7;
    } else {
      viewData_->songX_ = 7;
      clipboard_.x_ = 0;
    }
    isDirty_ = true;
  } else {
    if (viewData_->songY_ < clipboard_.y_) {
      viewData_->songY_ = 0;
      clipboard_.y_ = 0x17;
    } else {
      clipboard_.y_ = 0;
      viewData_->songY_ = 0x17;
    }
    isDirty_ = true;
  }
}

/******************************************************
 OnFocus:
        called when current view is becoming active
 ******************************************************/

void SongView::OnFocus() { clipboard_.active_ = false; };

GUIRect SongView::getSelectionRect() {

  GUIRect selRect(clipboard_.x_, clipboard_.y_ + clipboard_.offset_,
                  viewData_->songX_,
                  viewData_->songY_ + viewData_->songOffset_);

  selRect.Normalize();
  return selRect;
}

/******************************************************
 fillClipboard:
        fill clipboard with current selection value
 ******************************************************/

void SongView::fillClipboardData() {

  // Clear current selection data

  if (!clipboard_.data_)
    SYS_FREE((void *)clipboard_.data_);

  // Prepare selection related information

  GUIRect selRect = getSelectionRect();

  // Set current selection  data

  clipboard_.width_ = selRect.Width() + 1;
  clipboard_.height_ = selRect.Height() + 1;

  clipboard_.data_ =
      (unsigned char *)SYS_MALLOC(clipboard_.width_ * clipboard_.height_);

  unsigned char *src = viewData_->song_->data_ + selRect.Left() +
                       SONG_CHANNEL_COUNT * selRect.Top();
  unsigned char *dst = clipboard_.data_;

  for (int j = 0; j < clipboard_.height_; j++) {
    for (int i = 0; i < clipboard_.width_; i++) {
      *dst++ = *src++;
    }
    src += (SONG_CHANNEL_COUNT - clipboard_.width_);
  }
};

/******************************************************
 copySelection:
        copy current selection to clipboard
 ******************************************************/

void SongView::copySelection() {

  fillClipboardData();
  clipboard_.active_ = false;
  viewMode_ = VM_NORMAL;
  viewData_->songX_ = saveX_;
  viewData_->songY_ = saveY_;
  viewData_->songOffset_ = saveOffset_;
  isDirty_ = true;
}

/******************************************************
 cutSelection:
        cut current selection to clipboard
 ******************************************************/

void SongView::cutSelection() {

  // first copy the data to clipboard

  fillClipboardData();
  GUIRect selRect = getSelectionRect();

  // now move all rows up for cut

  unsigned char *dst = viewData_->song_->data_ + selRect.Left() +
                       SONG_CHANNEL_COUNT * (selRect.Top());
  unsigned char *src = dst + SONG_CHANNEL_COUNT * clipboard_.height_;

  int rowCount = SONG_ROW_COUNT - selRect.Bottom() - 1;

  for (int j = 0; j < rowCount; j++) {

    for (int i = 0; i < clipboard_.width_; i++) {
      *dst++ = *src++;
    }
    src += (SONG_CHANNEL_COUNT - clipboard_.width_);
    dst += (SONG_CHANNEL_COUNT - clipboard_.width_);
  }

  for (int j = 0; j > clipboard_.height_; j++) {
    for (int i = 0; i < clipboard_.width_; i++) {
      *dst++ = 0xFF;
    }
    dst += (SONG_CHANNEL_COUNT - clipboard_.width_);
  };

  clipboard_.active_ = false;
  viewMode_ = VM_NORMAL;
  viewData_->songX_ = saveX_;
  viewData_->songY_ = saveY_;
  viewData_->songOffset_ = saveOffset_;

  isDirty_ = true;
}

/******************************************************
 pasteSelection:
        paste clipboard content to song
 ******************************************************/

void SongView::pasteClipboard() {

  if (!clipboard_.data_)
    return;

  // Check we're not out of scope

  int width = clipboard_.width_;
  int height = clipboard_.height_;

  if (viewData_->songX_ + width > SONG_CHANNEL_COUNT) {
    width = SONG_CHANNEL_COUNT - viewData_->songX_;
  }
  if (viewData_->songY_ + viewData_->songOffset_ + height > SONG_ROW_COUNT) {
    height = SONG_ROW_COUNT - viewData_->songY_ - viewData_->songOffset_;
  } else {

    // Move down from insert point

    unsigned char *dst = viewData_->song_->data_ + viewData_->songX_ +
                         (SONG_ROW_COUNT - 1) * SONG_CHANNEL_COUNT;
    unsigned char *src = dst - height * SONG_CHANNEL_COUNT;

    int rowCount =
        SONG_ROW_COUNT - (viewData_->songY_ + viewData_->songOffset_);

    for (int j = 0; j < rowCount; j++) {
      for (int i = 0; i < width; i++) {
        *dst++ = *src++;
      }
      dst -= (SONG_CHANNEL_COUNT + width);
      src -= (SONG_CHANNEL_COUNT + width);
    }
  }

  // Prepare copy pointer

  unsigned char *dst = viewData_->GetCurrentSongPointer();
  unsigned char *src = clipboard_.data_;

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      *dst++ = *src++;
    }
    dst += (SONG_CHANNEL_COUNT - width);
    src += (clipboard_.width_ - width);
  }

  updateCursor(0, height);
}

void SongView::unMuteAll() {

  UIController *controller = UIController::GetInstance();
  controller->UnMuteAll();
};

void SongView::toggleMute() {

  UIController *controller = UIController::GetInstance();

  int from = viewData_->songX_;
  int to = from;
  if (clipboard_.active_) {
    GUIRect r = getSelectionRect();
    from = r.Left();
    to = r.Right();
  };
  controller->ToggleMute(from, to);
  viewMode_ = (viewMode_ != VM_MUTEON) ? VM_MUTEON : VM_NORMAL;
};

void SongView::switchSoloMode() {

  UIController *controller = UIController::GetInstance();
  int from = viewData_->songX_;
  int to = from;
  if (clipboard_.active_) {
    GUIRect r = getSelectionRect();
    from = r.Left();
    to = r.Right();
  };
  controller->SwitchSoloMode(from, to, (viewMode_ != VM_SOLOON));
  viewMode_ = (viewMode_ != VM_SOLOON) ? VM_SOLOON : VM_NORMAL;
  isDirty_ = true;
};

void SongView::onStart() {
  Player *player = Player::GetInstance();
  unsigned char from = viewData_->songX_;
  unsigned char to = from;
  if (clipboard_.active_) {
    GUIRect r = getSelectionRect();
    from = r.Left();
    to = r.Right();
  }
  player->OnSongStartButton(from, to, false, false);
};

void SongView::startCurrentRow() {
  Player *player = Player::GetInstance();
  player->SetSequencerMode(SM_LIVE);
  player->OnSongStartButton(0, 7, false, false);
}

void SongView::startImmediate() {
  Player *player = Player::GetInstance();

  unsigned char from = viewData_->songX_;
  unsigned char to = from;
  player->OnSongStartButton(from, to, false, true);
}

void SongView::onStop() {
  Player *player = Player::GetInstance();
  unsigned char from = viewData_->songX_;
  unsigned char to = from;
  if (clipboard_.active_) {
    GUIRect r = getSelectionRect();
    from = r.Left();
    to = r.Right();
  }
  player->OnSongStartButton(from, to, true, false);
};

void SongView::jumpToNextSection(int direction) {

  int current = viewData_->songY_ + viewData_->songOffset_;
  bool foundGap = false;
  for (int i = 0; i < SONG_ROW_COUNT; i++) {
    unsigned char *start = viewData_->song_->data_ + viewData_->songX_ +
                           SONG_CHANNEL_COUNT * current;
    if (foundGap && (*start != 0xFF)) {
      break;
    } else {
      if (*start == 0xFF) {
        foundGap = true;
      }
    }
    current += direction;
    if (current < 0) {
      current += SONG_ROW_COUNT;
    }
    if (current >= SONG_ROW_COUNT) {
      current -= SONG_ROW_COUNT;
    }
  }
  // If we go backwards, we stil have to go to the beginning of the block

  if (direction < 0) {
    while (current > 0) {
      unsigned char *start = viewData_->song_->data_ + viewData_->songX_ +
                             SONG_CHANNEL_COUNT * current;
      if (*start == 0xFF) {
        current++;
        break;
      };
      current--;
    };
  }

  // Update viewdata position from current

  if ((current - viewData_->songOffset_ > 0x17) ||
      (current - viewData_->songOffset_ < 0)) {
    viewData_->songOffset_ = current - 4;
    if (viewData_->songOffset_ < 0) {
      viewData_->songOffset_ = 0;
    }
  }
  viewData_->songY_ = current - viewData_->songOffset_;
  isDirty_ = true;
}

/******************************************************
 ProcessButtonMask:
        process button mask even coming from the main
        application window
 ******************************************************/

void SongView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed) {
    if (viewMode_ == VM_MUTEON) {
      if (mask & EPBM_R) {
        toggleMute();
      }
    };
    if (viewMode_ == VM_SOLOON) {
      if (mask & EPBM_R) {
        switchSoloMode();
      }
    };
    return;
  };

  if (viewMode_ == VM_NEW) {
    if (mask == EPBM_A) {
      unsigned short next = viewData_->song_->chain_.GetNext();
      if (next != NO_MORE_CHAIN) {
        setChain((unsigned char)next);
        isDirty_ = true;
      }
      mask &= (0xFFFF - EPBM_A);
    }
  }

  if (viewMode_ == VM_CLONE) {
    if ((mask & EPBM_A) && (mask & EPBM_L)) {
      clonePosition();
      mask &= (0xFFFF - (EPBM_A | EPBM_L));
    } else {
      viewMode_ = VM_SELECTION;
    }
  };

  if (clipboard_.active_) {
    viewMode_ = VM_SELECTION;
  };
  // Process selection related keys

  if (viewMode_ == VM_SELECTION) {
    if (clipboard_.active_ == false) {
      clipboard_.active_ = true;
      clipboard_.x_ = viewData_->songX_;
      clipboard_.y_ = viewData_->songY_;
      clipboard_.offset_ = viewData_->songOffset_;
      saveX_ = clipboard_.x_;
      saveY_ = clipboard_.y_;
      saveOffset_ = clipboard_.offset_;
    }
    processSelectionButtonMask(mask);
  } else {

    // Switch back to normal mode

    viewMode_ = VM_NORMAL;
    processNormalButtonMask(mask);
  }
}

/******************************************************
 processNormalButtonMask:
        process button mask in the case there is no
        selection active
 ******************************************************/

void SongView::processNormalButtonMask(unsigned int mask) {

  // B Modifier

  if (mask & EPBM_B) {
    if (mask & EPBM_DOWN)
      updateSongOffset(View::songRowCount_);
    if (mask & EPBM_UP)
      updateSongOffset(-View::songRowCount_);
    if (mask & (EPBM_RIGHT | EPBM_LEFT)) {
      Player *player = Player::GetInstance();
      switch (player->GetSequencerMode()) {
      case SM_SONG:
        player->SetSequencerMode(SM_LIVE);
        break;
      case SM_LIVE:
        player->SetSequencerMode(SM_SONG);
        break;
      }
      isDirty_ = true;
    }
    if ((mask & EPBM_A) && (!(mask & EPBM_R)))
      cutPosition();
    if (mask & EPBM_L) {
      viewMode_ = VM_CLONE;
    };
    if (mask & EPBM_R) {
      toggleMute();
    };
    if (mask & EPBM_START) {
      startImmediate();
    }
  } else {

    // A modifier

    if (mask & EPBM_A) {

      if (mask & EPBM_DOWN)
        updateChain(-0x10);
      if (mask & EPBM_UP)
        updateChain(0x10);
      if (mask & EPBM_LEFT)
        updateChain(-0x01);
      if (mask & EPBM_RIGHT)
        updateChain(0x01);
      if (mask & EPBM_L)
        pasteClipboard();
      if (mask == EPBM_A) {
        pasteLast();
        viewMode_ = VM_NEW;
      }
      if (mask & EPBM_R) {
        switchSoloMode();
      };
    } else {

      // R Modifier

      if (mask & EPBM_R) {

        if (mask & EPBM_L) {
          unMuteAll();
        }

        if (mask & EPBM_RIGHT) {
          unsigned char *data = viewData_->GetCurrentSongPointer();
          if (*data != 0xFF) {
            ViewType vt = VT_CHAIN;
            ViewEvent ve(VET_SWITCH_VIEW, &vt);
            viewData_->currentChain_ = *data;
            SetChanged();
            NotifyObservers(&ve);
          }
        }

        if (mask & EPBM_UP) {
          ViewType vt = VT_PROJECT;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_DOWN) {
          ViewType vt = VT_MIXER;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_START) {
          onStop();
        }

      } else {

        // L Modifier

        if (mask & EPBM_L) {
          if (mask & EPBM_DOWN)
            jumpToNextSection(1);
          if (mask & EPBM_UP)
            jumpToNextSection(-1);
          if (mask & EPBM_START)
            startCurrentRow();
        } else {

          // No modifier

          if (mask & EPBM_DOWN)
            updateCursor(0, 1);
          if (mask & EPBM_UP)
            updateCursor(0, -1);
          if (mask & EPBM_LEFT)
            updateCursor(-1, 0);
          if (mask & EPBM_RIGHT)
            updateCursor(1, 0);

          if (mask & EPBM_START) {
            onStart();
          }
        }
      }
    }
  }

  if ((!(mask & EPBM_A)) && updatingChain_) {
    unsigned char *c = viewData_->song_->data_ + updateX_ +
                       SONG_CHANNEL_COUNT * (viewData_->songOffset_ + updateY_);
    viewData_->song_->chain_.SetUsed(*c);
    updatingChain_ = false;
  }
};

/******************************************************
 processSelectionButtonMask:
        process button mask in the case there is a
        selection active
 ******************************************************/

void SongView::processSelectionButtonMask(unsigned int mask) {

  // B Modifier

  if (mask & EPBM_B) {
    if (mask & EPBM_R) {
      toggleMute();
    };
    if (mask & EPBM_L) {
      extendSelection();
    };
    if (mask == EPBM_B) {
      copySelection();
    }

  } else {

    // A modifier

    if (mask & EPBM_A) {
      if (mask & EPBM_L) {
        cutSelection();
      }
      if (mask & EPBM_R) {
        switchSoloMode();
      };
    } else {

      // R Modifier

      if (mask & EPBM_R) {

        if (mask & EPBM_L) {
          unMuteAll();
        }

        if (mask & EPBM_RIGHT) {
          unsigned char *data = viewData_->GetCurrentSongPointer();
          if (*data != 0xFF) {
            ViewType vt = VT_CHAIN;
            ViewEvent ve(VET_SWITCH_VIEW, &vt);
            viewData_->currentChain_ = *data;
            SetChanged();
            NotifyObservers(&ve);
          }
        }

        if (mask & EPBM_UP) {
          ViewType vt = VT_PROJECT;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_DOWN) {
          ViewType vt = VT_MIXER;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_START) {
          onStop();
        }

      } else {

        // No modifier

        if (mask & EPBM_DOWN)
          updateCursor(0, 1);
        if (mask & EPBM_UP)
          updateCursor(0, -1);
        if (mask & EPBM_LEFT)
          updateCursor(-1, 0);
        if (mask & EPBM_RIGHT)
          updateCursor(1, 0);
        if (mask & EPBM_START) {
          onStart();
        }
      }
    }
  }
}

/******************************************************
 Redraw:
        redraw completely the song view
 ******************************************************/

void SongView::DrawView() {

  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Prepare selection related information

  GUIRect selRect;
  if (clipboard_.active_) {
    selRect =
        GUIRect(clipboard_.x_, clipboard_.y_ + clipboard_.offset_,
                viewData_->songX_, viewData_->songY_ + viewData_->songOffset_);

    selRect.Normalize();
  }

  // Draw title
  SetColor(CD_NORMAL);

  Player *player = Player::GetInstance();

  props.invert_ = true;
  const char *buffer =
      ((player->GetSequencerMode() == SM_SONG) ? "Song" : "Live");
  DrawString(pos._x, pos._y, buffer, props);

  props.invert_ = false;
  std::string songname = songname_;
  if (songname.length() > 20) {
    songname = songname_.substr(0, 17) + "...";
  }
  DrawString(pos._x + 5, pos._y, songname.c_str(), props);

  // Compute song grid location
  GUIPoint anchor = GetAnchor();

  // draw battery gauge
  GUIPoint battpos = GetAnchor();
  battpos._y = 0;
  battpos._x = 26;
  System *sys = System::GetInstance();
  float batt = sys->GetBatteryLevel() / 1000.0;
  drawBattery(batt, battpos, props);

  // Display row numbers
  SetColor(CD_HILITE1);
  char row[3];
  pos = anchor;
  pos._x -= 3;
  for (int j = 0; j < View::songRowCount_; j++) {
    char p = j + viewData_->songOffset_;
    hex2char(p, row);
    DrawString(pos._x, pos._y, row, props);
    pos._y += 1;
  }

  SetColor(CD_NORMAL);

  pos = anchor;
  unsigned char *data =
      viewData_->song_->data_ + (SONG_CHANNEL_COUNT * viewData_->songOffset_);
  short dx = 3;
  short dy = 1;
  for (int j = 0; j < View::songRowCount_; j++) {

    pos._x = anchor._x;

    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {

      bool invert = false;

      // see if we need to invert current step
      // if there's a selection or we are at cursor position

      if (clipboard_.active_) {
        if ((i >= selRect.Left()) && (i <= selRect.Right()) &&
            (j + viewData_->songOffset_ >= selRect.Top()) &&
            (j + viewData_->songOffset_ <= selRect.Bottom())) {
          invert = true;
        }
      } else {
        if (i == viewData_->songX_ && j == viewData_->songY_) {
          invert = true;
        }
      }

      if (invert) {
        SetColor(CD_HILITE2);
        props.invert_ = true;
      }

      // draw current step

      unsigned char d = *data++;
      if (d == 0xFF) {
        DrawString(pos._x, pos._y, "--", props);
      } else {
        hex2char(d, row);
        DrawString(pos._x, pos._y, row, props);
      }

      // Put back drawing state

      if (invert) {
        SetColor(CD_NORMAL);
        props.invert_ = false;
      }

      // Next step

      pos._x += dx;
    }
    pos._y += dy;
  }
  SetColor(CD_NORMAL);

  drawMap();
  drawNotes();

  if (player->IsRunning()) {
    OnPlayerUpdate(PET_UPDATE);
  };
};

/******************************************************
 OnPlayterUpdate:
        Called when positions in player change. Should
        provide visual feedback of currently played
        position
 ******************************************************/

void SongView::OnPlayerUpdate(PlayerEventType eventType, unsigned int tick) {

  SyncMaster *sync = SyncMaster::GetInstance();
  if ((eventType == PET_UPDATE) && (!sync->MajorSlice()))
    return;

  Player *player = Player::GetInstance();

  GUIPoint anchor = GetAnchor();
  GUIPoint pos = anchor;
  pos._x -= 1;

  GUITextProperties props;
  SetColor(CD_CURSOR);

  // Loop on all channels

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {

    // Clear all current positions

    int y = lastPlayedPosition_[i] - viewData_->songOffset_;
    if (y >= 0 && y < View::songRowCount_) {
      pos._y = anchor._y + y;
      DrawString(pos._x, pos._y, " ", props);
    }

    // Clear all last queued positions

    y = lastQueuedPosition_[i] - viewData_->songOffset_;
    if (y >= 0 && y < View::songRowCount_) {
      pos._y = anchor._y + y;
      DrawString(pos._x, pos._y, " ", props);
    }

    // For each playing position, draw current location

    if (player->IsChannelPlaying(i)) {
      if (eventType != PET_STOP) {
        if (viewData_->currentPlayChain_[i] != 0xFF) {
          int y = viewData_->songPlayPos_[i] - viewData_->songOffset_;
          if (y >= 0 && y < View::songRowCount_ &&
              viewData_->playMode_ != PM_AUDITION) {
            pos._y = anchor._y + y;
            if (!player->IsChannelMuted(i)) {
              DrawString(pos._x, pos._y, ">", props);
            } else {
              DrawString(pos._x, pos._y, "-", props);
            }
            lastPlayedPosition_[i] = viewData_->songPlayPos_[i];
          }
        }
      }
    }

    // If in live mode, update queued position

    if (player->GetSequencerMode() == SM_LIVE) {
      if (player->GetQueueingMode(i) != QM_NONE) {

        if (eventType != PET_STOP) {
          int y = player->GetQueuePosition(i) - viewData_->songOffset_;
          if (y >= 0 && y < View::songRowCount_) {
            pos._y = anchor._y + y;
            const char *indicator = player->GetLiveIndicator(i);
            DrawString(pos._x, pos._y, indicator, props);
            lastQueuedPosition_[i] = player->GetQueuePosition(i);
          }
        }
      };
    }
    pos._x += 3;
  }

  SetColor(CD_NORMAL);

  // Draw clipping indicator
  if (View::miniLayout_) {
    pos._y = 0;
    pos._x = 25;
  } else {
    pos = 0;
    pos._x = 26;
  }

  pos._y += 1; // make space for batt guage which is on top line
  if (player->Clipped()) {
    DrawString(pos._x, pos._y, "clip", props);
  } else {
    DrawString(pos._x, pos._y, "----", props);
  }

  char strbuffer[10];

  if (eventType != PET_STOP) {
    SetColor(CD_NORMAL);
    props.invert_ = false;
    int time = int(player->GetPlayTime());
    int mi = time / 60;
    int se = time - mi * 60;
    sprintf(strbuffer, "%2.2d:%2.2d", mi, se);
    pos._y += 1;
    DrawString(pos._x, pos._y, strbuffer, props);
  }

  drawNotes();

  // re-draw the VU meter
  drawMasterVuMeter(player, pos, props);
};

void SongView::AnimationUpdate() {
  // redraw batt gauge on every clock tick (~1Hz) even when not playing
  // and not redrawing due to user cursor navigation
  GUIPoint anchor = GetAnchor();
  GUIPoint pos = anchor;
  GUITextProperties props;
  pos._y = 0;
  pos._x = 26;

  System *sys = System::GetInstance();
  float batt = sys->GetBatteryLevel() / 1000.0;
  drawBattery(batt, pos, props);
  w_.Flush();
};