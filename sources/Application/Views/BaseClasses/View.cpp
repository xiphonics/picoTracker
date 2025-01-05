#include "View.h"
#include "Application/AppWindow.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "ModalView.h"
#include "System/Console/Trace.h"

bool View::initPrivate_ = false;

int View::margin_ = 0;
int View::songRowCount_; //=21 ;

View::View(GUIWindow &w, ViewData *viewData)
    : w_(w), viewData_(viewData), viewMode_(VM_NORMAL) {
  if (!initPrivate_) {
    View::margin_ = 0;
    songRowCount_ = 16;

    initPrivate_ = true;
  }
  mask_ = 0;
  locked_ = false;
  modalView_ = 0;
  modalViewCallback_ = 0;
  hasFocus_ = false;
};

GUIPoint View::GetAnchor() {
  // Original code had a dynamic anchor point dending on song count, but
  // changing the song count didn't work anyway given that there are many places
  // where it was statically defined as 8. Other screens also don't fit with a
  // dynamic anchor point
  return GUIPoint(5, 3);
}

GUIPoint View::GetTitlePosition() {
#ifndef PLATFORM_CAANOO
  return GUIPoint(0, 0);
#else
  return GUIPoint(0, 1);
#endif
};

bool View::Lock() {
  if (locked_)
    return false;
  locked_ = true;
  return true;
};

void View::WaitForObject() {
  while (locked_) {
  };
}

void View::Unlock() { locked_ = false; }

void View::drawMap() {
  GUIPoint anchor = GetAnchor();
  GUIPoint pos(View::margin_, anchor._y + View::songRowCount_ + 1);
  GUITextProperties props;

  // draw entire map
  SetColor(CD_NORMAL);
  char buffer[5];
  props.invert_ = false;
  // row1
  strcpy(buffer, "D   ");
  DrawString(pos._x, pos._y, buffer, props);
  pos._y++;
  // row2
  strcpy(buffer, "P G ");
  DrawString(pos._x, pos._y, buffer, props);
  pos._y++;
  // row3
  strcpy(buffer, "SCPI");
  DrawString(pos._x, pos._y, buffer, props);
  pos._y++;
  // row4
  strcpy(buffer, "  TT");
  DrawString(pos._x, pos._y, buffer, props);

  // draw current screen on map
  SetColor(CD_HILITE2);
  pos._y = anchor._y + View::songRowCount_ + 1;
  switch (viewType_) {
  case VT_CHAIN:
    pos._x += 1;
    pos._y += 2;
    DrawString(pos._x, pos._y, "C", props);
    break;
  case VT_PHRASE:
    pos._x += 2;
    pos._y += 2;
    DrawString(pos._x, pos._y, "P", props);
    break;
  case VT_DEVICE:
    DrawString(pos._x, pos._y, "D", props);
    break;
  case VT_PROJECT:
    pos._y += 1;
    DrawString(pos._x, pos._y, "P", props);
    break;
  case VT_INSTRUMENT:
    pos._x += 3;
    pos._y += 2;
    DrawString(pos._x, pos._y, "I", props);
    break;
  case VT_TABLE: // under phrase
    pos._x += 2;
    pos._y += 3;
    DrawString(pos._x, pos._y, "T", props);
    break;
  case VT_TABLE2: // under instrument
    pos._x += 3;
    pos._y += 3;
    DrawString(pos._x, pos._y, "T", props);
    break;
  case VT_GROOVE:
    pos._x += 2;
    pos._y += 1;
    DrawString(pos._x, pos._y, "G", props);
    break;
  default: // VT_SONG
    pos._y += 2;
    DrawString(pos._x, pos._y, "S", props);
  }
}

void View::drawNotes() {

  GUIPoint anchor = GetAnchor();
  int initialX = View::margin_ + 5;
  int initialY = anchor._y + View::songRowCount_ + 2;
  GUIPoint pos(initialX, initialY);
  GUITextProperties props;

  Player *player = Player::GetInstance();

  // column banger refactor
  props.invert_ = true;
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    if (i == viewData_->songX_) {
      SetColor(CD_HILITE2);
    } else {
      SetColor(CD_HILITE1);
    }
    if (player->IsRunning() && viewData_->playMode_ != PM_AUDITION) {
      DrawString(pos._x, pos._y, player->GetPlayedNote(i),
                 props); // row for the note values
      pos._y++;
      DrawString(pos._x, pos._y, player->GetPlayedOctive(i),
                 props); // row for the octive values
      pos._y++;
      DrawString(pos._x, pos._y, player->GetPlayedInstrument(i),
                 props); // draw instrument number
    } else {
      DrawString(pos._x, pos._y, "  ", props); // row for the note
                                               // values
      pos._y++;
      DrawString(pos._x, pos._y, "  ",
                 props); // row for the octive values
      pos._y++;
      DrawString(pos._x, pos._y, "  ", props); // draw instrument number
    }
    pos._y = initialY;
    pos._x += 3;
  }
}

void View::DoModal(ModalView *view, ModalViewCallback cb) {
  modalView_ = view;
  modalView_->OnFocus();
  modalViewCallback_ = cb;
  isDirty_ = true;
};

void View::Redraw() {
  if (modalView_) {
    if (isDirty_) {
      DrawView();
    }
    modalView_->Redraw();
  } else {
    DrawView();
  }
  isDirty_ = false;
};

void View::SetDirty(bool isDirty) { isDirty_ = true; };

void View::ProcessButton(unsigned short mask, bool pressed) {
  isDirty_ = false;
  if (modalView_) {
    modalView_->ProcessButton(mask, pressed);
    if (modalView_->IsFinished()) {
      // process callback sending the modal dialog
      if (modalViewCallback_) {
        modalViewCallback_(*this, *modalView_);
      }
      SAFE_DELETE(modalView_);
      isDirty_ = true;
    }
  } else {
    ProcessButtonMask(mask, pressed);
  }
  if (isDirty_)
    ((AppWindow &)w_).SetDirty();
};

void View::Clear() { ((AppWindow &)w_).Clear(); }

void View::SetColor(ColorDefinition cd) { ((AppWindow &)w_).SetColor(cd); };

void View::ClearRect(int x, int y, int w, int h) {
  GUIRect rect(x, y, (x + w), (y + h));
  w_.ClearRect(rect);
};

void View::DrawString(int x, int y, const char *txt, GUITextProperties &props) {
  GUIPoint pos(x, y);
  w_.DrawString(txt, pos, props);
};

void View::drawBattery(GUITextProperties &props) {
  GUIPoint battpos = GetAnchor();
  battpos._y = 0;
  battpos._x = 27;
  System *sys = System::GetInstance();
  float voltage = sys->GetBatteryLevel() / 1000.0;

  if (voltage >= 0) {
    SetColor(CD_INFO);

    char *battText;
    if (voltage > 4.0) {
      battText = (char *)"[CHG]";
    } else if (voltage > 3.7) {
      battText = (char *)"[+++]";
    } else if (voltage > 3.5) {
      battText = (char *)"[++ ]";
    } else if (voltage > 3.4) {
      SetColor(CD_WARN);
      battText = (char *)"[+  ]";
    } else {
      SetColor(CD_ERROR);
      battText = (char *)"[   ]";
    }

    DrawString(battpos._x, battpos._y, battText, props);
  }
}
