#include "PhraseView.h"
#include "Application/Instruments/CommandList.h"
#include "Application/Model/Scale.h"
#include "Application/Model/Table.h"
#include "Application/Utils/HelpLegend.h"
#include "Application/Utils/char.h"
#include "System/Console/Trace.h"
#include "UIController.h"
#include "ViewData.h"
#include <etl/string.h>
#include <stdlib.h>

short PhraseView::offsets_[2][4] = {-1, 1, 12, -12, -1, 1, 16, -16};

PhraseView::PhraseView(GUIWindow &w, ViewData *viewData)
    : View(w, viewData), cmdEdit_("edit", FCC_EDIT, 0) {
  phrase_ = &(viewData_->song_->phrase_);
  lastPlayingPos_ = 0;
  GUIPoint pos(0, 10);
  cmdEditField_ =
      new UIBigHexVarField(pos, cmdEdit_, 4, "%4.4X", 0, 0xFFFF, 16, true);
  row_ = 0;
  viewData->phraseCurPos_ = 0;
  col_ = 0;
  lastNote_ = 60;
  lastInstr_ = 0;
  lastCmd_ = I_CMD_NONE;
  lastParam_ = 0;

  clipboard_.active_ = false;
  clipboard_.width_ = 0;
  clipboard_.height_ = 0;

  for (int i = 0; i < 16; i++) {
    clipboard_.note_[i] = 0xFF;
    clipboard_.instr_[i] = 0;
  };
}

PhraseView::~PhraseView() { delete cmdEditField_; };

void PhraseView::updateCursor(int dx, int dy) {

  col_ += dx;
  row_ += dy;
  if (col_ > 5)
    col_ = 5;
  if (col_ < 0)
    col_ = 0;
  if (row_ > 15) {
    // Try to see if the current chain has a phrase after this one

    if ((viewMode_ != VM_SELECTION) && (viewData_->chainRow_ < 15)) {
      viewData_->chainRow_++;
      unsigned char *p = viewData_->GetCurrentChainPointer();
      if (*p != 0xFF) {
        viewData_->currentPhrase_ = *p;
        row_ = 0;
      } else { // rollback
        viewData_->chainRow_--;
        row_ = 15;
      }
    } else {
      row_ = 15;
    }
  }
  if (row_ < 0) {

    // Try to see if the current chain has a phrase before this one

    if ((viewMode_ != VM_SELECTION) && (viewData_->chainRow_ > 0)) {
      viewData_->chainRow_--;
      unsigned char *p = viewData_->GetCurrentChainPointer();
      if (*p != 0xFF) {
        viewData_->currentPhrase_ = *p;
        row_ = 15;
      } else { // rollback
        viewData_->chainRow_++;
        row_ = 0;
      }
    } else {
      row_ = 0;
    }
  }
  GUIPoint anchor = GetAnchor();
  GUIPoint p(anchor);
  switch (col_) {
  case 3:
    p._x += 12;
    p._y += row_;
    cmdEditField_->SetPosition(p);
    cmdEdit_.SetInt(
        *(phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_)));
    break;
  case 5:
    p._x += 21;
    p._y += row_;
    cmdEditField_->SetPosition(p);
    cmdEdit_.SetInt(
        *(phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_)));
    break;
  };

  viewData_->phraseCurPos_ = row_;
  isDirty_ = true;
}

void PhraseView::updateCursorValue(ViewUpdateDirection direction, int xOffset,
                                   int yOffset) {

  unsigned char *c = 0;
  unsigned char limit = 0;
  bool wrap = false;
  FourCC *cc;

  switch (col_ + xOffset) {
  case 0:
    c = phrase_->note_ + (16 * viewData_->currentPhrase_ + row_ + yOffset);
    limit = 119;
    wrap = true;
    break;
  case 1:
    c = phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_ + yOffset);
    limit = MAX_INSTRUMENT_COUNT - 1;
    wrap = true;
    break;
  case 2:
    cc = phrase_->cmd1_ + (16 * viewData_->currentPhrase_ + row_ + yOffset);
    switch (direction) {
    case VUD_RIGHT:
      *cc = CommandList::GetNext(*cc);
      break;
    case VUD_UP:
      *cc = CommandList::GetNextAlpha(*cc);
      break;
    case VUD_LEFT:
      *cc = CommandList::GetPrev(*cc);
      break;
    case VUD_DOWN:
      *cc = CommandList::GetPrevAlpha(*cc);
      break;
    }
    lastCmd_ = *cc;
    break;

  case 3:
    switch (direction) {
    case VUD_RIGHT:
      cmdEditField_->ProcessArrow(EPBM_RIGHT);
      break;
    case VUD_UP:
      cmdEditField_->ProcessArrow(EPBM_UP);
      break;
    case VUD_LEFT:
      cmdEditField_->ProcessArrow(EPBM_LEFT);
      break;
    case VUD_DOWN:
      cmdEditField_->ProcessArrow(EPBM_DOWN);
      break;
    }
    *(phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_ + yOffset)) =
        cmdEdit_.GetInt();
    lastParam_ = cmdEdit_.GetInt();
    break;
  case 4:
    cc = phrase_->cmd2_ + (16 * viewData_->currentPhrase_ + row_ + yOffset);
    switch (direction) {
    case VUD_RIGHT:
      *cc = CommandList::GetNext(*cc);
      break;
    case VUD_UP:
      *cc = CommandList::GetNextAlpha(*cc);
      break;
    case VUD_LEFT:
      *cc = CommandList::GetPrev(*cc);
      break;
    case VUD_DOWN:
      *cc = CommandList::GetPrevAlpha(*cc);
      break;
    }
    lastCmd_ = *cc;
    break;
  case 5:
    switch (direction) {
    case VUD_RIGHT:
      cmdEditField_->ProcessArrow(EPBM_RIGHT);
      break;
    case VUD_UP:
      cmdEditField_->ProcessArrow(EPBM_UP);
      break;
    case VUD_LEFT:
      cmdEditField_->ProcessArrow(EPBM_LEFT);
      break;
    case VUD_DOWN:
      cmdEditField_->ProcessArrow(EPBM_DOWN);
      break;
    }
    *(phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_ + yOffset)) =
        cmdEdit_.GetInt();
    lastParam_ = cmdEdit_.GetInt();
    break;
  }
  if ((c) && (*c != 0xFF)) {
    int offset = offsets_[col_ + xOffset][direction];

    // Add/remove from offset to match selected scale
    int scale = viewData_->project_->GetScale();
    while (!scaleSteps[scale][(*c + offset) % 12]) {
      offset > 0 ? offset++ : offset--;
    }

    updateData(c, offset, limit, wrap);
    switch (col_ + xOffset) {
    case 0:
      lastNote_ = *c;
      break;
    case 1:
      lastInstr_ = *c;
      break;
    }
  }
  isDirty_ = true;
}

// If we're on an empty spot, we past the last element
// otherwise we take the current phrase as last

void PhraseView::pasteLast() {

  uchar *c = 0;

  switch (col_) {
  case 0:
    c = phrase_->note_ + (16 * viewData_->currentPhrase_ + row_);
    if ((*c == 0xFF)) {
      *c = lastNote_;
      c = phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_);
      *c = lastInstr_;
      isDirty_ = true;
    } else {
      lastNote_ = *c;
      c = phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_);
      lastInstr_ = *c;
    }
    break;
  case 1:
    c = phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_);
    if ((*c == 0xFF)) {
      *c = lastInstr_;
      isDirty_ = true;
    } else {
      lastInstr_ = *c;
    }
    break;
  case 2:
    c = phrase_->cmd1_ + (16 * viewData_->currentPhrase_ + row_);
    if (*c == I_CMD_NONE) {
      *c = lastCmd_;
      isDirty_ = true;
    } else {
      lastCmd_ = *c;
    }
    break;

  case 3:
    /*			s=phrase_->param1_+(16*viewData_->currentPhrase_+row_) ;
                            if (*s==0) {
                                    *s=lastParam_ ;
                                    cmdEdit_.SetInt(lastParam_) ;
                                    isDirty_=true ;
                            }
    �*/
    break;

  case 4:
    c = phrase_->cmd2_ + (16 * viewData_->currentPhrase_ + row_);
    if (*c == I_CMD_NONE) {
      *c = lastCmd_;
      isDirty_ = true;
    } else {
      lastCmd_ = *c;
    }
    break;

  case 5:
    /*			s=phrase_->param2_+(16*viewData_->currentPhrase_+row_) ;
                            if (*s==0) {
                                    *s=lastParam_ ;
                                    isDirty_=true ;
                                    cmdEdit_.SetInt(lastParam_) ;
                            }
    */
    break;
  }
}

void PhraseView::cutPosition() {

  clipboard_.active_ = true;
  clipboard_.row_ = row_;
  clipboard_.col_ = col_;
  saveRow_ = row_;
  saveCol_ = col_;

  if (col_ % 2 == 0)
    col_ += 1; // This way, A+B on note cuts
               // the instruments too and parameters get cut with commands
  cutSelection();
};

void PhraseView::warpInChain(int offset) {

  int currentRow = viewData_->chainRow_;
  viewData_->chainRow_ += offset;
  if ((viewData_->chainRow_ < 16) && (viewData_->chainRow_ >= 0)) {
    unsigned char *p = viewData_->GetCurrentChainPointer();
    if (*p != 0xFF) {
      viewData_->currentPhrase_ = *p;
      switch (col_) {
      case 3:
        cmdEdit_.SetInt(
            *(phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_)));
        break;
      case 5:
        cmdEdit_.SetInt(
            *(phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_)));
        break;
      };
    } else { // rollback
      viewData_->chainRow_ = currentRow;
    }
  } else { // rollback
    viewData_->chainRow_ = currentRow;
  }
  isDirty_ = true;
}

void PhraseView::warpToNeighbour(int offset) {
  int newPos = viewData_->songX_ + offset;
  if ((newPos > -1) && (newPos < SONG_CHANNEL_COUNT)) {
    // Go to neighbout song channel
    viewData_->songX_ = newPos;
    unsigned char *c = viewData_->GetCurrentSongPointer();
    // is there a chain ?
    unsigned char oldChain = viewData_->currentChain_;
    if (*c != 0xFF) {
      // go to chain
      viewData_->currentChain_ = *c;
      // get phrase at location
      unsigned char *p = viewData_->GetCurrentChainPointer();
      // is there a phrase ?
      if (*p != 0xFF) {
        viewData_->currentPhrase_ = *p;
        updateCursor(0, 0);
        isDirty_ = true;
      } else { // restore chain & song
        viewData_->currentChain_ = oldChain;
        viewData_->songX_ -= offset;
      }
    } else { // restore song
      viewData_->songX_ -= offset;
    }
  }
}

/******************************************************
 getSelectionRect:
        gets the normalized rectangle of the current
        selection. Valid only while selection is drawn
 ******************************************************/

GUIRect PhraseView::getSelectionRect() {
  GUIRect r(clipboard_.col_, clipboard_.row_, col_, row_);
  r.Normalize();
  return r;
};

/******************************************************
 fillClipboardData:

        copies the necessary information from the
        current selection to the clipboard for future
        paste. We're copying data all across the row
        because we"re too lazy to try to figure a better
        procedure
 ******************************************************/

void PhraseView::fillClipboardData() {

  // Get Current normalized selection rect

  GUIRect selRect = getSelectionRect();

  // Get size & store in clipboard

  clipboard_.width_ = selRect.Width() + 1;
  clipboard_.height_ = selRect.Height() + 1;
  clipboard_.row_ = selRect.Top();
  clipboard_.col_ = selRect.Left();

  // Copy the data

  uchar *src1 =
      viewData_->song_->phrase_.note_ + 16 * viewData_->currentPhrase_;
  uchar *dst1 = clipboard_.note_;
  uchar *src2 =
      viewData_->song_->phrase_.instr_ + 16 * viewData_->currentPhrase_;
  uchar *dst2 = clipboard_.instr_;
  uchar *src3 =
      viewData_->song_->phrase_.cmd1_ + 16 * viewData_->currentPhrase_;
  uchar *dst3 = clipboard_.cmd1_;
  ushort *src4 =
      viewData_->song_->phrase_.param1_ + 16 * viewData_->currentPhrase_;
  ushort *dst4 = clipboard_.param1_;
  uchar *src5 =
      viewData_->song_->phrase_.cmd2_ + 16 * viewData_->currentPhrase_;
  uchar *dst5 = clipboard_.cmd2_;
  ushort *src6 =
      viewData_->song_->phrase_.param2_ + 16 * viewData_->currentPhrase_;
  ushort *dst6 = clipboard_.param2_;

  for (int i = 0; i < clipboard_.height_; i++) {
    dst1[i] = src1[clipboard_.row_ + i];
    dst2[i] = src2[clipboard_.row_ + i];
    dst3[i] = src3[clipboard_.row_ + i];
    dst4[i] = src4[clipboard_.row_ + i];
    dst5[i] = src5[clipboard_.row_ + i];
    dst6[i] = src6[clipboard_.row_ + i];
  };
  updateCursor(0, 0);
};

void PhraseView::updateSelectionValue(ViewUpdateDirection direction) { // HERE

  saveRow_ = row_;
  saveCol_ = col_;

  GUIRect r = getSelectionRect();
  col_ = r.Left();
  row_ = r.Top();

  for (int i = 0; i <= r.Width(); i++) {
    for (int j = 0; j <= r.Height(); j++) {
      if (col_ + i < 2) {
        updateCursorValue(direction, i, j);
      }
    }
  }
  row_ = saveRow_;
  col_ = saveCol_;
}

void PhraseView::extendSelection() {
  GUIRect rect = getSelectionRect();
  if (rect.Left() > 0 || rect.Right() < 5) {
    if (col_ < clipboard_.col_) {
      col_ = 0;
      clipboard_.col_ = 5;
    } else {
      col_ = 5;
      clipboard_.col_ = 0;
    }
    isDirty_ = true;
  } else {
    if (row_ < clipboard_.row_) {
      row_ = 0;
      clipboard_.row_ = 15;
    } else {
      clipboard_.row_ = 0;
      row_ = 15;
    }
    isDirty_ = true;
  }
}
/******************************************************
 copySelection:
        copies data in the current selection to the
        clipboard & end selection process
 ******************************************************/

void PhraseView::copySelection() {

  // Keep up with row,col of selection coz
  // fillClipboardData will trash it

  fillClipboardData();

  clipboard_.active_ = false;
  viewMode_ = VM_NORMAL;
  row_ = saveRow_;
  col_ = saveCol_;

  isDirty_ = true;
};

/******************************************************
 cut:  copies data in the current selection to the
       clipboard, clear selection content & end selection
       process
 ******************************************************/

void PhraseView::cutSelection() {

  // Keep up with row,col of selection coz
  // fillClipboardData will trash it

  fillClipboardData();

  // Loop over selection col, row & clear data inside it

  uchar *dst1 =
      viewData_->song_->phrase_.note_ + 16 * viewData_->currentPhrase_;
  uchar *dst2 =
      viewData_->song_->phrase_.instr_ + 16 * viewData_->currentPhrase_;
  uchar *dst3 =
      viewData_->song_->phrase_.cmd1_ + 16 * viewData_->currentPhrase_;
  ushort *dst4 =
      viewData_->song_->phrase_.param1_ + 16 * viewData_->currentPhrase_;
  uchar *dst5 =
      viewData_->song_->phrase_.cmd2_ + 16 * viewData_->currentPhrase_;
  ushort *dst6 =
      viewData_->song_->phrase_.param2_ + 16 * viewData_->currentPhrase_;

  for (int i = 0; i < clipboard_.width_; i++) {
    for (int j = 0; j < clipboard_.height_; j++) {
      switch (i + clipboard_.col_) {
      case 0:
        dst1[j + clipboard_.row_] = 0xFF;
        break;
      case 1:
        dst2[j + clipboard_.row_] = 0xFF;
        break;
      case 2:
        dst3[j + clipboard_.row_] = I_CMD_NONE;
        break;
      case 3:
        dst4[j + clipboard_.row_] = 0x0000;
        break;
      case 4:
        dst5[j + clipboard_.row_] = I_CMD_NONE;
        break;
      case 5:
        dst6[j + clipboard_.row_] = 0x0000;
        break;
      }
    }
  }

  // Clear selection, end selection process & reposition cursor

  clipboard_.active_ = false;
  viewMode_ = VM_NORMAL;
  row_ = saveRow_;
  col_ = saveCol_;
  updateCursor(0, 0);
  isDirty_ = true;
};

/******************************************************
 pasteClipboard:
        copies data in the clipboard to the current step
 ******************************************************/

void PhraseView::pasteClipboard() {

  // Get number of row to paste

  int height = clipboard_.height_;
  /*    if (row_+height>16) {
          height=16-row_ ;
      }
    */
  uchar *dst1 =
      viewData_->song_->phrase_.note_ + 16 * viewData_->currentPhrase_;
  uchar *src1 = clipboard_.note_;
  uchar *dst2 =
      viewData_->song_->phrase_.instr_ + 16 * viewData_->currentPhrase_;
  uchar *src2 = clipboard_.instr_;
  uchar *dst3 =
      viewData_->song_->phrase_.cmd1_ + 16 * viewData_->currentPhrase_;
  uchar *src3 = clipboard_.cmd1_;
  ushort *dst4 =
      viewData_->song_->phrase_.param1_ + 16 * viewData_->currentPhrase_;
  ushort *src4 = clipboard_.param1_;
  uchar *dst5 =
      viewData_->song_->phrase_.cmd2_ + 16 * viewData_->currentPhrase_;
  uchar *src5 = clipboard_.cmd2_;
  ushort *dst6 =
      viewData_->song_->phrase_.param2_ + 16 * viewData_->currentPhrase_;
  ushort *src6 = clipboard_.param2_;

  for (int i = 0; i < clipboard_.width_; i++) {
    for (int j = 0; j < height; j++) {
      switch (i + clipboard_.col_) {
      case 0:
        dst1[(j + row_) % 16] = src1[j];
        break;
      case 1:
        dst2[(j + row_) % 16] = src2[j];
        break;
      case 2:
        dst3[(j + row_) % 16] = src3[j];
        break;
      case 3:
        dst4[(j + row_) % 16] = src4[j];
        break;
      case 4:
        dst5[(j + row_) % 16] = src5[j];
        break;
      case 5:
        dst6[(j + row_) % 16] = src6[j];
        break;
      }
    }
  }
  int offset = (row_ + height) % 16 - row_;
  updateCursor(0x00, offset);
  isDirty_ = true;
};

void PhraseView::stopAudition() {
  Player *player = Player::GetInstance();
  if (viewData_->playMode_ == PM_AUDITION)
    player->Stop();
}

void PhraseView::unMuteAll() {

  UIController *controller = UIController::GetInstance();
  controller->UnMuteAll();
};

void PhraseView::toggleMute() {

  UIController *controller = UIController::GetInstance();
  controller->ToggleMute(viewData_->songX_, viewData_->songX_);
  viewMode_ = (viewMode_ != VM_MUTEON) ? VM_MUTEON : VM_NORMAL;
};

void PhraseView::switchSoloMode() {

  UIController *controller = UIController::GetInstance();
  controller->SwitchSoloMode(viewData_->songX_, viewData_->songX_,
                             (viewMode_ == VM_NORMAL));
  viewMode_ = (viewMode_ != VM_SOLOON) ? VM_SOLOON : VM_NORMAL;
  isDirty_ = true;
};

void PhraseView::OnFocus() {
  clipboard_.active_ = false;
  viewMode_ = VM_NORMAL;
  updateCursor(0, 0);
};

void PhraseView::ProcessButtonMask(unsigned short mask, bool pressed) {

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

      // If note or I, we request a new instr

      if (col_ < 2) {
        InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
        unsigned short next = bank->GetNext();
        if (next != NO_MORE_INSTRUMENT) {
          unsigned char *c =
              phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_);
          *c = (unsigned char)next;
          lastInstr_ = next;
          isDirty_ = true;
        }
        mask &= (0xFFFF - EPBM_A);
      } else {
        if ((col_ == 3) &&
            (*(phrase_->cmd1_ + (16 * viewData_->currentPhrase_ + row_))) ==
                I_CMD_TABL) {
          TableHolder *th = TableHolder::GetInstance();
          unsigned short next = th->GetNext();
          if (next != NO_MORE_TABLE) {
            ushort *c =
                phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_);
            *c = next;
            isDirty_ = true;
            mask &= (0xFFFF - EPBM_A);
            cmdEdit_.SetInt(next);
          }
        }
        if ((col_ == 5) &&
            (*(phrase_->cmd2_ + (16 * viewData_->currentPhrase_ + row_))) ==
                I_CMD_TABL) {
          TableHolder *th = TableHolder::GetInstance();
          unsigned short next = th->GetNext();
          if (next != NO_MORE_TABLE) {
            ushort *c =
                phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_);
            *c = next;
            isDirty_ = true;
            mask &= (0xFFFF - EPBM_A);
            cmdEdit_.SetInt(next);
          }
        }
      };
    }
  }

  if (viewMode_ == VM_CLONE) {
    if ((mask & EPBM_A) && (mask & EPBM_L)) {
      if (col_ < 2) {
        InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
        unsigned char *c =
            phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_);
        if (*c != 0xFF) {
          unsigned short next = bank->Clone(*c);
          if (next != NO_MORE_INSTRUMENT) {
            *c = (unsigned char)next;
            lastInstr_ = next;
            isDirty_ = true;
          }
        }
      } else {
        if ((col_ == 3) &&
            (*(phrase_->cmd1_ + (16 * viewData_->currentPhrase_ + row_))) ==
                I_CMD_TABL) {
          TableHolder *th = TableHolder::GetInstance();
          int current =
              *(phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_));
          if (current != -1) {
            unsigned short next = th->Clone(current);
            if (next != NO_MORE_TABLE) {
              ushort *c =
                  phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_);
              *c = next;
              isDirty_ = true;
              cmdEdit_.SetInt(next);
            }
          }
        }
        if ((col_ == 5) &&
            (*(phrase_->cmd2_ + (16 * viewData_->currentPhrase_ + row_))) ==
                I_CMD_TABL) {
          TableHolder *th = TableHolder::GetInstance();
          unsigned short next = th->Clone(
              *(phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_)));
          if (next != NO_MORE_TABLE) {
            ushort *c =
                phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_);
            *c = next;
            isDirty_ = true;
            cmdEdit_.SetInt(next);
          }
        }
      };
      mask &= (0xFFFF - (EPBM_A | EPBM_L));
    } else {
      viewMode_ = VM_SELECTION;
    }
  };

  if (viewMode_ == VM_SELECTION) {
    if (!clipboard_.active_) {
      clipboard_.active_ = true;
      clipboard_.col_ = col_;
      clipboard_.row_ = row_;
      saveCol_ = col_;
      saveRow_ = row_;
    }
    processSelectionButtonMask(mask);
  } else {
    viewMode_ = VM_NORMAL;
    processNormalButtonMask(mask);
  };
}

void PhraseView::processNormalButtonMask(unsigned short mask) {

  // B Modifier

  Player *player = Player::GetInstance();

  if (mask & EPBM_B) {
    if (mask & EPBM_LEFT)
      warpToNeighbour(-1);
    if (mask & EPBM_RIGHT)
      warpToNeighbour(1);
    if (mask & EPBM_UP)
      warpInChain(-1);
    if (mask & EPBM_DOWN)
      warpInChain(1);
    if (mask & EPBM_A) {
      stopAudition();
      cutPosition();
    }
    if (mask & EPBM_L) {
      viewMode_ = VM_CLONE;
    };
    if (mask & EPBM_R)
      toggleMute();
    if (mask == EPBM_B) {
      stopAudition();
    }
  } else {

    // A Modifer

    if (mask & EPBM_A) {
      if (mask & EPBM_DOWN)
        updateCursorValue(VUD_DOWN);
      if (mask & EPBM_UP)
        updateCursorValue(VUD_UP);
      if (mask & EPBM_LEFT)
        updateCursorValue(VUD_LEFT);
      if (mask & EPBM_RIGHT)
        updateCursorValue(VUD_RIGHT);
      if (mask & EPBM_L)
        pasteClipboard();
      if (mask & EPBM_R)
        switchSoloMode();
      if (mask == EPBM_A) {
        pasteLast();
        if ((col_ == 1) || (col_ == 3) || (col_ == 5))
          viewMode_ = VM_NEW;
        if (col_ == 0 || col_ == 1) {
          if (player->IsRunning()) {
            if ((viewData_->playMode_ == PM_AUDITION)) {
              player->Stop();
              player->OnStartButton(PM_AUDITION, viewData_->songX_, false,
                                    viewData_->chainRow_);
            }
          } else {
            player->OnStartButton(PM_AUDITION, viewData_->songX_, false,
                                  viewData_->chainRow_);
          }
        }
      }
    } else {

      // R Modifier

      if (mask & EPBM_R) {
        if (mask & EPBM_LEFT) {
          stopAudition();
          ViewType vt = VT_CHAIN;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }
        if (mask & EPBM_RIGHT) {
          stopAudition();
          unsigned char *c =
              phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_);
          if (*c != 0xFF) {
            viewData_->currentInstrument_ = *c;
          } else {
            viewData_->currentInstrument_ = lastInstr_;
          }
          if (viewData_->currentInstrument_ != 0xFF) {
            ViewType vt = VT_INSTRUMENT;
            ViewEvent ve(VET_SWITCH_VIEW, &vt);
            SetChanged();
            NotifyObservers(&ve);
          }
        }
        if (mask & EPBM_DOWN) {
          // Go to table view
          stopAudition();

          ViewType vt = VT_TABLE;

          FourCC *cmd =
              phrase_->cmd1_ + (16 * viewData_->currentPhrase_ + row_);
          ushort *param =
              phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_);

          if (*cmd != I_CMD_TABL) {
            cmd = phrase_->cmd2_ + (16 * viewData_->currentPhrase_ + row_);
            param = phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_);
          }
          if (*cmd == I_CMD_TABL) {
            viewData_->currentTable_ = (*param) & (TABLE_COUNT - 1);
          }
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_UP) {
          // Go to groove view
          stopAudition();

          ViewType vt = VT_GROOVE;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }

        if (mask & EPBM_START) {
          player->OnStartButton(PM_PHRASE, viewData_->songX_, true,
                                viewData_->chainRow_);
        }
        if (mask & EPBM_L)
          unMuteAll();

      } else {
        // L Modifier
        if (mask & EPBM_L) {

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
            player->OnStartButton(PM_PHRASE, viewData_->songX_, false,
                                  viewData_->chainRow_);
          }
        }
      }
    }
  }
};

void PhraseView::processSelectionButtonMask(unsigned short mask) {

  Player *player = Player::GetInstance();

  // B modifier

  if (mask & EPBM_B) {
    if (mask & EPBM_L) {
      extendSelection();
    } else {
      copySelection();
    }
  } else {

    // A Modifer

    if (mask & EPBM_A) {

      if (mask & EPBM_DOWN)
        updateSelectionValue(VUD_DOWN);
      if (mask & EPBM_UP)
        updateSelectionValue(VUD_UP);
      if (mask & EPBM_LEFT)
        updateSelectionValue(VUD_LEFT);
      if (mask & EPBM_RIGHT)
        updateSelectionValue(VUD_RIGHT);

      if (mask & EPBM_L)
        cutSelection();
      if (mask & EPBM_R)
        switchSoloMode();
    } else {

      // R Modifier

      if (mask & EPBM_R) {
        if (mask & EPBM_LEFT) {
          ViewType vt = VT_CHAIN;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }
        if (mask & EPBM_RIGHT) {
          unsigned char *c =
              phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_);
          if (*c != 0xFF) {
            viewData_->currentInstrument_ = *c;
          } else {
            viewData_->currentInstrument_ = lastInstr_;
          }
          ViewType vt = VT_INSTRUMENT;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }
        if (mask & EPBM_START) {
          player->OnStartButton(PM_PHRASE, viewData_->songX_, true,
                                viewData_->chainRow_);
        }
        if (mask & EPBM_L)
          unMuteAll();

      } else {
        // L Modifier
        if (mask & EPBM_L) {

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
            player->OnStartButton(PM_PHRASE, viewData_->songX_, false,
                                  viewData_->chainRow_);
          }
        }
      }
    }
  }
};

void PhraseView::setTextProps(GUITextProperties &props, int row, int col,
                              bool restore) {

  bool invert = false;

  if (clipboard_.active_) {
    GUIRect selRect = getSelectionRect();
    if ((row >= selRect.Left()) && (row <= selRect.Right()) &&
        (col >= selRect.Top()) && (col <= selRect.Bottom())) {
      invert = true;
    }
  } else {
    if ((col_ == row) && (row_ == col)) {
      invert = true;
    }
  }

  if (invert) {
    if (restore) {
      SetColor(CD_NORMAL);
      props.invert_ = false;
    } else {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    }
  }
};

void PhraseView::DrawView() {

  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title

  char title[20];

  SetColor(CD_NORMAL);
  sprintf(title, "Phrase %2.2x", viewData_->currentPhrase_);
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

  SetColor(CD_NORMAL);

  pos = anchor;

  // Display notes

  unsigned char *data = phrase_->note_ + (16 * viewData_->currentPhrase_);

  buffer[4] = 0;
  for (int j = 0; j < 16; j++) {
    unsigned char d = *data++;
    setTextProps(props, 0, j, false);
    if (d == 0xFF) {
      DrawString(pos._x, pos._y, "----", props);
    } else {
      note2char(d, buffer);
      DrawString(pos._x, pos._y, buffer, props);
    }
    setTextProps(props, 0, j, true);
    pos._y++;
  }

  // Draw instruments

  pos = anchor;
  pos._x += 4;

  data = phrase_->instr_ + (16 * viewData_->currentPhrase_);
  buffer[0] = 'I';
  buffer[3] = 0;

  for (int j = 0; j < 16; j++) {
    unsigned char d = *data++;
    setTextProps(props, 1, j, false);
    if (d == 0xFF) {
      DrawString(pos._x, pos._y, "I--", props);
    } else {
      hex2char(d, buffer + 1);
      DrawString(pos._x, pos._y, buffer, props);
      if (j == row_) {
        sprintf(buffer, "I%2.2x: ", d);
        etl::string<32> instrLine = buffer;
        setTextProps(props, 1, j, true);
        GUIPoint location = GetTitlePosition();
        location._x += 12;
        InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
        I_Instrument *instr = bank->GetInstrument(d);
        instrLine += instr->GetName();
        DrawString(location._x, location._y, instrLine.c_str(), props);
      }
    }
    setTextProps(props, 1, j, true);
    pos._y++;
  }

  // Draw command 1

  pos = anchor;
  pos._x += 8;

  FourCC *f = phrase_->cmd1_ + (16 * viewData_->currentPhrase_);

  buffer[4] = 0;

  for (int j = 0; j < 16; j++) {
    FourCC command = *f++;
    fourCC2char(command, buffer);
    setTextProps(props, 2, j, false);
    DrawString(pos._x, pos._y, buffer, props);
    setTextProps(props, 2, j, true);
    pos._y++;
    if (j == row_ && (col_ == 2 || col_ == 3)) {
      printHelpLegend(command, props);
    }
  }

  // Draw commands params 1

  pos = anchor;
  pos._x += 12;

  ushort *param = phrase_->param1_ + (16 * viewData_->currentPhrase_);
  buffer[5] = 0;

  for (int j = 0; j < 16; j++) {
    ushort p = *param++;
    setTextProps(props, 3, j, false);
    /*		if (p==0xFFFF) {
                            DrawString(pos._x,pos._y,"----",props) ;
                    } else {
    */
    hexshort2char(p, buffer);
    DrawString(pos._x, pos._y, buffer, props);
    /*		}
     */
    setTextProps(props, 3, j, true);
    pos._y++;
  }

  // Draw commands 2

  pos = anchor;
  pos._x += 17;

  f = phrase_->cmd2_ + (16 * viewData_->currentPhrase_);

  buffer[4] = 0;

  for (int j = 0; j < 16; j++) {
    FourCC command = *f++;
    fourCC2char(command, buffer);
    setTextProps(props, 4, j, false);
    DrawString(pos._x, pos._y, buffer, props);
    setTextProps(props, 4, j, true);
    pos._y++;
    if (j == row_ && (col_ == 4 || col_ == 5)) {
      printHelpLegend(command, props);
    }
  }

  // Draw commands params

  pos = anchor;
  pos._x += 21;

  param = phrase_->param2_ + (16 * viewData_->currentPhrase_);
  buffer[5] = 0;

  for (int j = 0; j < 16; j++) {
    ushort p = *param++;
    setTextProps(props, 5, j, false);
    /*		if (p==0xFFFF) {
                            DrawString(pos._x,pos._y,"----",props) ;
                    } else {
    */
    hexshort2char(p, buffer);
    DrawString(pos._x, pos._y, buffer, props);
    /*		}
     */
    setTextProps(props, 5, j, true);
    pos._y++;
  }

  drawMap();
  drawNotes();

  Player *player = Player::GetInstance();
  if (player->IsRunning()) {
    OnPlayerUpdate(PET_UPDATE);
  };

  if ((viewMode_ != VM_SELECTION) && ((col_ == 3) || (col_ == 5))) {
    cmdEditField_->SetFocus();
    cmdEditField_->Draw(w_);
  };
};

void PhraseView::OnPlayerUpdate(PlayerEventType eventType, unsigned int tick) {

  drawNotes();

  GUIPoint anchor = GetAnchor();
  GUIPoint pos = anchor;
  pos._x -= 1;

  GUITextProperties props;
  SetColor(CD_NORMAL);

  pos._y = anchor._y + lastPlayingPos_;
  DrawString(pos._x, pos._y, " ", props);

  Player *player = Player::GetInstance();

  if (eventType != PET_STOP) {

    // Clear current position if needed

    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      if (player->IsChannelPlaying(i)) {

        if (viewData_->currentPlayPhrase_[i] == viewData_->currentPhrase_ &&
            viewData_->playMode_ != PM_AUDITION) {
          pos._y = anchor._y + viewData_->phrasePlayPos_[i];
          if (!player->IsChannelMuted(i)) {
            DrawString(pos._x, pos._y, ">", props);
          } else {
            DrawString(pos._x, pos._y, "-", props);
          }
          lastPlayingPos_ = viewData_->phrasePlayPos_[i];
          break;
        }
      }
    }

    // clear any live indicator

    pos._y = anchor._y;
    DrawString(pos._x, pos._y, " ", props);

    // Loop on all channels to see if one has queued current chain
    if (player->GetSequencerMode() == SM_LIVE) {

      for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
        // is anything queued ?
        if (player->GetQueueingMode(i) != QM_NONE) {
          // find the chain queued in channel
          unsigned char songPos = player->GetQueuePosition(i);
          unsigned char *chain =
              viewData_->song_->data_ + i + SONG_CHANNEL_COUNT * songPos;
          if (*chain == viewData_->currentChain_) {
            const char *indicator = player->GetLiveIndicator(i);
            DrawString(pos._x, pos._y, indicator, props);
            break;
          }
        }
      }
    }
  }

  pos = anchor;
  pos._x += 200;

  // re-draw the VU meter
  drawMasterVuMeter(player, pos, props);

  /*	if (player->Clipped()) {
             w_.DrawString("clip",pos,props);
      } else {
             w_.DrawString("----",pos,props);
      }
  */
};

void PhraseView::printHelpLegend(FourCC command, GUITextProperties props) {
  char **helpLegend = getHelpLegend(command);
  DrawString(0, 0, helpLegend[0], props);
  DrawString(5, 1, helpLegend[1], props);
}
