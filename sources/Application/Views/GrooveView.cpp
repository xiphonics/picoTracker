
#include "GrooveView.h"
#include "Application/Model/Groove.h"
#include "Application/Utils/char.h"
#include "ViewData.h"

GrooveView::GrooveView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {
  position_ = 0;
  lastPosition_ = 0;
}

GrooveView::~GrooveView() {}

void GrooveView::updateCursor(int dir) {
  position_ += dir;
  if (position_ < 0)
    position_ += 16;
  if (position_ > 15)
    position_ -= 16;
  isDirty_ = true;
};

void GrooveView::updateCursorValue(int val, bool sync) {
  unsigned char *grooveData =
      Groove::GetInstance()->GetGrooveData(viewData_->currentGroove_);
  int value = grooveData[position_];
  val += value;
  if (val < 1)
    val = 1;
  if (val > 0xF)
    val = 0xF;
  grooveData[position_] = val;
  isDirty_ = true;
};

void GrooveView::warpGroove(int dir) {
  int current = viewData_->currentGroove_;
  current += dir;
  if (current >= MAX_GROOVES) {
    current -= MAX_GROOVES;
  };
  if (current < 0) {
    current += MAX_GROOVES;
  };
  viewData_->currentGroove_ = current;
  isDirty_ = true;
};

void GrooveView::initCursorValue() {
  unsigned char *grooveData =
      Groove::GetInstance()->GetGrooveData(viewData_->currentGroove_);
  if (grooveData[position_] == NO_GROOVE_DATA) {
    grooveData[position_] = 1;
  };
  isDirty_ = true;
};

void GrooveView::clearCursorValue() {
  unsigned char *grooveData =
      Groove::GetInstance()->GetGrooveData(viewData_->currentGroove_);
  grooveData[position_] = NO_GROOVE_DATA;
  isDirty_ = true;
}

void GrooveView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed)
    return;

  Player *player = Player::GetInstance();

  if (mask & EPBM_B) {
    if (mask & EPBM_LEFT) {
      warpGroove(-1);
    }
    if (mask & EPBM_RIGHT) {
      warpGroove(1);
    }
    if (mask & EPBM_DOWN) {
      warpGroove(-0x10);
    }
    if (mask & EPBM_UP) {
      warpGroove(0x10);
    }
    if (mask & EPBM_A) {
      clearCursorValue();
    };
  } else {

    // A modifier
    if (mask & EPBM_A) {
      if (mask & EPBM_LEFT) {
        updateCursorValue(-1);
      }
      if (mask & EPBM_RIGHT) {
        updateCursorValue(1);
      }
      if (mask & EPBM_DOWN) {
        updateCursorValue(-1, true);
      }
      if (mask & EPBM_UP) {
        updateCursorValue(1, true);
      }
      if (mask == EPBM_A) {
        initCursorValue();
      };
    } else {
      // R Modifier

      if (mask & EPBM_R) {
        if (mask & EPBM_DOWN) {
          ViewType vt = VT_PHRASE;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }
        if (mask & EPBM_START) {
          player->OnStartButton(PM_PHRASE, viewData_->songX_, true,
                                viewData_->chainRow_);
        }

      } else {
        // No modifier
        if (mask & EPBM_DOWN)
          updateCursor(1);
        if (mask & EPBM_UP)
          updateCursor(-1);
        if (mask & EPBM_START) {
          player->OnStartButton(PM_PHRASE, viewData_->songX_, false,
                                viewData_->chainRow_);
        }
      }
    }
  }
};

void GrooveView::DrawView() {

  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title

  char title[40];

  SetColor(CD_NORMAL);

  sprintf(title, "Groove: %2.2x", viewData_->currentGroove_);
  DrawString(pos._x, pos._y, title, props);

  // Compute song grid location

  GUIPoint anchor = GetAnchor();

  // Display row numbers

  SetColor(CD_HILITE1);
  char buffer[6];
  pos = anchor;
  pos._x -= 3;
  for (int j = 0; j < 16; j++) {
    hex2char(j, buffer);
    DrawString(pos._x, pos._y, buffer, props);
    pos._y++;
  }

  // Display current groove

  pos = anchor;

  SetColor(CD_NORMAL);

  unsigned char *grooveData =
      Groove::GetInstance()->GetGrooveData(viewData_->currentGroove_);
  for (int j = 0; j < 16; j++) {
    if (grooveData[j] != NO_GROOVE_DATA) {
      hex2char(grooveData[j], buffer);
      buffer[3] = 0;
    } else {
      strcpy(buffer, "--");
    };
    props.invert_ = (j == position_);
    DrawString(pos._x, pos._y, buffer, props);
    pos._y++;
  }

  drawMap();
  drawNotes();
};

void GrooveView::OnPlayerUpdate(PlayerEventType, unsigned int tick) {

  GUITextProperties props;
  GUIPoint anchor = GetAnchor();
  GUIPoint pos;

  pos._x = anchor._x - 1;
  pos._y = anchor._y + lastPosition_;
  DrawString(pos._x, pos._y, " ", props);

  Groove *gr = Groove::GetInstance();
  // Get current channel
  int channel = viewData_->songX_;

  int groove;
  int groovepos;

  gr->GetChannelData(channel, &groove, &groovepos);

  if (groove == viewData_->currentGroove_ &&
      viewData_->playMode_ != PM_AUDITION) {
    lastPosition_ = groovepos;
    pos._x = anchor._x - 1;
    pos._y = anchor._y + lastPosition_;
    DrawString(pos._x, pos._y, ">", props);
  };

  drawNotes();

  Player *player = Player::GetInstance();
  //  re-draw the VU meter
  drawMasterVuMeter(player, pos, props);
};

void GrooveView::OnFocus(){};
