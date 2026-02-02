/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "PhraseView.h"
#include "Application/Instruments/CommandList.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Model/Scale.h"
#include "Application/Model/Table.h"
#include "Application/Utils/HelpLegend.h"
#include "Application/Utils/char.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/SampleEditorView.h"
#include "System/Console/Trace.h"
#include "UIController.h"
#include "ViewData.h"
#include <Application/AppWindow.h>
#include <cstdint>
#include <etl/string.h>
#include <nanoprintf.h>
#include <stdlib.h>

short PhraseView::offsets_[2][4] = {-1, 1, 12, -12, -1, 1, 16, -16};

PhraseView::PhraseView(GUIWindow &w, ViewData *viewData)
    : ScreenView(w, viewData), cmdEdit_(FourCC::ActionEdit, 0),
      cmdEditPos_(0, 10),
      cmdEditField_(cmdEditPos_, cmdEdit_, 4, "%4.4X", 0, 0xFFFF, 16, true) {
  phrase_ = &(viewData_->song_->phrase_);
  lastPlayingPos_ = 0;
  row_ = 0;
  viewData->phraseCurPos_ = 0;
  col_ = 0;
  lastNote_ = 60;
  lastInstr_ = 0;
  lastCmd_ = FourCC::InstrumentCommandNone;
  lastParam_ = 0;

  clipboard_.active_ = false;
  clipboard_.width_ = 0;
  clipboard_.height_ = 0;

  for (int i = 0; i < 16; i++) {
    clipboard_.note_[i] = NO_NOTE;
    clipboard_.instr_[i] = 0;
  };
}

PhraseView::~PhraseView(){};

bool PhraseView::getEffectiveInstrumentForRow(int row,
                                              uint8_t &instrumentId) const {
  if (!phrase_) {
    return false;
  }
  if (row < 0) {
    return false;
  }
  unsigned char *instrData = phrase_->instr_ + (16 * viewData_->currentPhrase_);
  for (int i = row; i >= 0; --i) {
    unsigned char instr = instrData[i];
    if (instr != 0xFF) {
      instrumentId = instr;
      return true;
    }
  }
  return false;
}

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
    cmdEditField_.SetPosition(p);
    cmdEdit_.SetInt(
        *(phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_)));
    break;
  case 5:
    p._x += 21;
    p._y += row_;
    cmdEditField_.SetPosition(p);
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
    limit = HIGHEST_NOTE;
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

  case 3: {
    switch (direction) {
    case VUD_RIGHT:
      cmdEditField_.ProcessArrow(EPBM_RIGHT);
      break;
    case VUD_UP:
      cmdEditField_.ProcessArrow(EPBM_UP);
      break;
    case VUD_LEFT:
      cmdEditField_.ProcessArrow(EPBM_LEFT);
      break;
    case VUD_DOWN:
      cmdEditField_.ProcessArrow(EPBM_DOWN);
      break;
    }
    // Sanitize MIDI velocity values if needed
    FourCC currentCmd =
        *(phrase_->cmd1_ + (16 * viewData_->currentPhrase_ + row_ + yOffset));
    ushort paramValue = cmdEdit_.GetInt();
    paramValue = CommandList::RangeLimitCommandParam(currentCmd, paramValue);
    cmdEdit_.SetInt(paramValue);
    *(phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_ + yOffset)) =
        paramValue;
    lastParam_ = paramValue;
    break;
  }
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
      cmdEditField_.ProcessArrow(EPBM_RIGHT);
      break;
    case VUD_UP:
      cmdEditField_.ProcessArrow(EPBM_UP);
      break;
    case VUD_LEFT:
      cmdEditField_.ProcessArrow(EPBM_LEFT);
      break;
    case VUD_DOWN:
      cmdEditField_.ProcessArrow(EPBM_DOWN);
      break;
    }
    // Sanitize MIDI velocity values if needed
    FourCC currentCmd =
        *(phrase_->cmd2_ + (16 * viewData_->currentPhrase_ + row_ + yOffset));
    ushort paramValue = cmdEdit_.GetInt();
    paramValue = CommandList::RangeLimitCommandParam(currentCmd, paramValue);
    cmdEdit_.SetInt(paramValue);
    *(phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_ + yOffset)) =
        paramValue;
    lastParam_ = paramValue;
    break;
  }
  if ((c) && (*c != NO_NOTE)) {
    int offset = offsets_[col_ + xOffset][direction];

    // if note column apply the set scale or slice range
    if (col_ + xOffset == 0) {
      uint8_t instrId = 0;
      InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
      SampleInstrument *sliceInstr = nullptr;
      if (bank && getEffectiveInstrumentForRow(row_ + yOffset, instrId)) {
        I_Instrument *instr = bank->GetInstrument(instrId);
        if (instr && instr->GetType() == IT_SAMPLE) {
          sliceInstr = static_cast<SampleInstrument *>(instr);
        }
      }

      uint8_t sliceFirst = 0;
      uint8_t sliceLast = 0;
      if (sliceInstr && sliceInstr->GetSliceNoteRange(sliceFirst, sliceLast)) {
        int newNote = *c + offset;
        if (newNote < sliceFirst) {
          newNote = sliceFirst;
        } else if (newNote > sliceLast) {
          newNote = sliceLast;
        }
        *c = static_cast<unsigned char>(newNote);
      } else {
        // Add/remove from offset to match selected scale
        int scale = viewData_->project_->GetScale();
        int scaleRoot = viewData_->project_->GetScaleRoot();

        // Calculate the new note with the offset
        int newNote = *c + offset;

        // Check if the note is in the scale (adjusted for root)
        // For root = 0, (newNote + 12 - 0) % 12 simplifies to newNote % 12
        while (newNote >= 0 &&
               !scaleSteps[scale][(newNote + 12 - scaleRoot) % 12]) {
          offset > 0 ? offset++ : offset--;
          newNote = *c + offset;
        }
        updateData(c, offset, limit, wrap);
      }
    } else {
      updateData(c, offset, limit, wrap);
    }

    switch (col_ + xOffset) {
    case 0: {
      lastNote_ = *c;

      // Need to restart audition to update it with the new note
      startAudition(false);
      break;
    }
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
    if ((*c == NO_NOTE)) {
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
    c = (unsigned char *)phrase_->cmd1_ +
        (16 * viewData_->currentPhrase_ + row_);
    if (*c == FourCC::InstrumentCommandNone) {
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
    c = (unsigned char *)phrase_->cmd2_ +
        (16 * viewData_->currentPhrase_ + row_);
    if (*c == FourCC::InstrumentCommandNone) {
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
  // cutting an empty note slot adds a note off
  uint8_t *note = phrase_->note_ + (16 * viewData_->currentPhrase_ + row_);
  if (col_ == 0 && *note == NO_NOTE) {
    *note = NOTE_OFF;
    isDirty_ = true;
    return;
  }

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
  uchar *src3 = (unsigned char *)viewData_->song_->phrase_.cmd1_ +
                16 * viewData_->currentPhrase_;
  uchar *dst3 = clipboard_.cmd1_;
  ushort *src4 =
      viewData_->song_->phrase_.param1_ + 16 * viewData_->currentPhrase_;
  ushort *dst4 = clipboard_.param1_;
  uchar *src5 = (unsigned char *)viewData_->song_->phrase_.cmd2_ +
                16 * viewData_->currentPhrase_;
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
  uchar *dst3 = (unsigned char *)viewData_->song_->phrase_.cmd1_ +
                16 * viewData_->currentPhrase_;
  ushort *dst4 =
      viewData_->song_->phrase_.param1_ + 16 * viewData_->currentPhrase_;
  uchar *dst5 = (unsigned char *)viewData_->song_->phrase_.cmd2_ +
                16 * viewData_->currentPhrase_;
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
        dst3[j + clipboard_.row_] = FourCC::InstrumentCommandNone;
        break;
      case 3:
        dst4[j + clipboard_.row_] = 0x0000;
        break;
      case 4:
        dst5[j + clipboard_.row_] = FourCC::InstrumentCommandNone;
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
  uchar *dst3 = (unsigned char *)viewData_->song_->phrase_.cmd1_ +
                16 * viewData_->currentPhrase_;
  uchar *src3 = clipboard_.cmd1_;
  ushort *dst4 =
      viewData_->song_->phrase_.param1_ + 16 * viewData_->currentPhrase_;
  ushort *src4 = clipboard_.param1_;
  uchar *dst5 = (unsigned char *)viewData_->song_->phrase_.cmd2_ +
                16 * viewData_->currentPhrase_;
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

inline void PhraseView::startAudition(bool startIfNotRunning) {
  Player *player = Player::GetInstance();
  if (player->IsRunning()) {
    // now also update if in auditioning mode
    if (viewData_->playMode_ == PM_AUDITION) {
      player->Stop();
      player->OnStartButton(PM_AUDITION, viewData_->songX_, false,
                            viewData_->chainRow_);
    }
  } else if (startIfNotRunning) {
    player->OnStartButton(PM_AUDITION, viewData_->songX_, false,
                          viewData_->chainRow_);
  }
}

inline void PhraseView::stopAudition() {
  Player *player = Player::GetInstance();
  if (viewData_->playMode_ == PM_AUDITION) {
    player->Stop();
  }
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
    // ENTER might now no longer be pressed so first check if we were in
    // audition mode and if its not then stop auditioning, stopAudition does
    // both those things
    if (!(mask & EPBM_ENTER)) {
      stopAudition();
    }

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

  if (viewMode_ == VM_NEW) {
    if (mask == EPBM_ENTER) {
      // If note or I, we request a new instr
      if (col_ < 2) {
        InstrumentBank *bank = viewData_->project_->GetInstrumentBank();

        auto nextId = bank->GetNextFreeInstrumentSlotId();
        // New Instruments default to type NONE!
        unsigned short next = bank->GetNextAndAssignID(IT_NONE, nextId);
        if (next != NO_MORE_INSTRUMENT) {
          unsigned char *c =
              phrase_->instr_ + (16 * viewData_->currentPhrase_ + row_);
          *c = (unsigned char)next;
          lastInstr_ = next;
          isDirty_ = true;
        } else {
          // show error dialog that no more instruments are available
          MessageBox *mb =
              MessageBox::Create(*this, "No more instruments!", MBBF_OK);
          DoModal(mb);
          return;
        }
        mask &= (0xFFFF - EPBM_ENTER);
      } else {
        if ((col_ == 3) &&
            (*(phrase_->cmd1_ + (16 * viewData_->currentPhrase_ + row_))) ==
                FourCC::SampleInstrumentTable) {
          TableHolder *th = TableHolder::GetInstance();
          unsigned short next = th->GetNext();
          if (next != NO_MORE_TABLE) {
            ushort *c =
                phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_);
            *c = next;
            isDirty_ = true;
            mask &= (0xFFFF - EPBM_ENTER);
            cmdEdit_.SetInt(next);
          }
        }
        if ((col_ == 5) &&
            (*(phrase_->cmd2_ + (16 * viewData_->currentPhrase_ + row_))) ==
                FourCC::SampleInstrumentTable) {
          TableHolder *th = TableHolder::GetInstance();
          unsigned short next = th->GetNext();
          if (next != NO_MORE_TABLE) {
            ushort *c =
                phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_);
            *c = next;
            isDirty_ = true;
            mask &= (0xFFFF - EPBM_ENTER);
            cmdEdit_.SetInt(next);
          }
        }
      };
    }
  }

  if (viewMode_ == VM_CLONE) {
    if ((mask & EPBM_ENTER) && (mask & EPBM_ALT)) {
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
                FourCC::SampleInstrumentTable) {
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
                FourCC::SampleInstrumentTable) {
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
      mask &= (0xFFFF - (EPBM_ENTER | EPBM_ALT));
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
  Player *player = Player::GetInstance();

  if (mask & EPBM_EDIT) {
    // EDIT Modifier
    if (mask & EPBM_LEFT)
      warpToNeighbour(-1);
    if (mask & EPBM_RIGHT)
      warpToNeighbour(1);
    if (mask & EPBM_UP)
      warpInChain(-1);
    if (mask & EPBM_DOWN)
      warpInChain(1);
    if (mask & EPBM_ENTER) {
      cutPosition();
    }
    if (mask & EPBM_ALT) {
      viewMode_ = VM_CLONE;
    };
    if (mask & EPBM_NAV)
      toggleMute();
    if (mask & EPBM_PLAY) {
      // recording screen
      if (!Player::GetInstance()->IsRunning()) {
        switchToRecordView();
      }
    }
  } else if (mask & EPBM_ENTER) {
    // ENTER Modifer
    if (mask & EPBM_DOWN)
      updateCursorValue(VUD_DOWN);
    if (mask & EPBM_UP)
      updateCursorValue(VUD_UP);
    if (mask & EPBM_LEFT)
      updateCursorValue(VUD_LEFT);
    if (mask & EPBM_RIGHT)
      updateCursorValue(VUD_RIGHT);
    if (mask & EPBM_ALT)
      pasteClipboard();
    if (mask & EPBM_NAV)
      switchSoloMode();
    if (mask == EPBM_ENTER) {
      pasteLast();
      if ((col_ == 1) || (col_ == 3) || (col_ == 5))
        viewMode_ = VM_NEW;
      if (col_ == 0 || col_ == 1) {
        // Start auditionq, note stopping audition happens in
        // processButtonMask on key up
        stopAudition();
        startAudition(true);
      }
    }
  } else if (mask & EPBM_NAV) {
    // NAV Modifier
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
        viewData_->currentInstrumentID_ = *c;
      } else {
        viewData_->currentInstrumentID_ = lastInstr_;
      }
      if (viewData_->currentInstrumentID_ != 0xFF) {
        ViewType vt = VT_INSTRUMENT;
        ViewEvent ve(VET_SWITCH_VIEW, &vt);
        SetChanged();
        NotifyObservers(&ve);
      }
    }
    if (mask & EPBM_DOWN) {
      // Go to table view

      ViewType vt = VT_TABLE;

      FourCC *cmd = phrase_->cmd1_ + (16 * viewData_->currentPhrase_ + row_);
      ushort *param =
          phrase_->param1_ + (16 * viewData_->currentPhrase_ + row_);

      if (*cmd != FourCC::SampleInstrumentTable) {
        cmd = phrase_->cmd2_ + (16 * viewData_->currentPhrase_ + row_);
        param = phrase_->param2_ + (16 * viewData_->currentPhrase_ + row_);
      }
      if (*cmd == FourCC::SampleInstrumentTable) {
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

    if (mask & EPBM_PLAY) {
      player->OnStartButton(PM_PHRASE, viewData_->songX_, true,
                            viewData_->chainRow_);
    }
    if (mask & EPBM_ALT)
      unMuteAll();

  } else if (mask & EPBM_ALT) {
    // ALT Modifier

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
    if (mask & EPBM_PLAY) {
      player->OnStartButton(PM_PHRASE, viewData_->songX_, false,
                            viewData_->chainRow_);
    }
  }
}

void PhraseView::processSelectionButtonMask(unsigned short mask) {

  Player *player = Player::GetInstance();

  // B modifier

  if (mask & EPBM_EDIT) {
    if (mask & EPBM_ALT) {
      extendSelection();
    } else {
      copySelection();
    }
  } else {

    // A Modifer

    if (mask & EPBM_ENTER) {

      if (mask & EPBM_DOWN)
        updateSelectionValue(VUD_DOWN);
      if (mask & EPBM_UP)
        updateSelectionValue(VUD_UP);
      if (mask & EPBM_LEFT)
        updateSelectionValue(VUD_LEFT);
      if (mask & EPBM_RIGHT)
        updateSelectionValue(VUD_RIGHT);

      if (mask & EPBM_ALT)
        cutSelection();
      if (mask & EPBM_NAV)
        switchSoloMode();
    } else {

      // R Modifier

      if (mask & EPBM_NAV) {
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
            viewData_->currentInstrumentID_ = *c;
          } else {
            viewData_->currentInstrumentID_ = lastInstr_;
          }
          ViewType vt = VT_INSTRUMENT;
          ViewEvent ve(VET_SWITCH_VIEW, &vt);
          SetChanged();
          NotifyObservers(&ve);
        }
        if (mask & EPBM_PLAY) {
          player->OnStartButton(PM_PHRASE, viewData_->songX_, true,
                                viewData_->chainRow_);
        }
        if (mask & EPBM_ALT)
          unMuteAll();

      } else {
        // L Modifier
        if (mask & EPBM_ALT) {

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
          if (mask & EPBM_PLAY) {
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

  char title[SCREEN_WIDTH + 1];

  SetColor(CD_NORMAL);
  npf_snprintf(title, sizeof(title), "Phrase %2.2x", viewData_->currentPhrase_);
  DrawString(pos._x, pos._y, title, props);

  // Compute song grid location

  GUIPoint anchor = GetAnchor();

  // Display row numbers

  SetColor(CD_HILITE1);
  char buffer[6];
  pos = anchor;
  pos._x -= 3;
  for (int j = 0; j < 16; j++) {
    ((j / ALT_ROW_NUMBER) % 2) ? SetColor(CD_ACCENT) : SetColor(CD_ACCENTALT);
    hex2char(j, buffer);
    DrawString(pos._x, pos._y, buffer, props);
    pos._y++;
  }

  SetColor(CD_NORMAL);

  pos = anchor;

  // Display notes
  unsigned char *data = phrase_->note_ + (16 * viewData_->currentPhrase_);
  unsigned char *instrData = phrase_->instr_ + (16 * viewData_->currentPhrase_);
  unsigned char lastInstr = 0xFF;
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();

  buffer[4] = 0;
  for (int j = 0; j < 16; j++) {
    unsigned char d = *data++;
    unsigned char instr = *instrData++;
    if (instr != 0xFF) {
      lastInstr = instr;
    }
    unsigned char effectiveInstr = lastInstr;
    setTextProps(props, 0, j, false);
    (0 == j || 4 == j || 8 == j || 12 == j) ? SetColor(CD_HILITE1)
                                            : SetColor(CD_NORMAL);
    if (d == NO_NOTE) {
      DrawString(pos._x, pos._y, "----", props);
    } else if (d == NOTE_OFF) {
      DrawString(pos._x, pos._y, "off ", props);
    } else {
      bool showSlice = false;
      bool invalidSlice = false;
      uint8_t sliceIndex = 0;
      if (effectiveInstr != 0xFF && bank) {
        I_Instrument *instrObj = bank->GetInstrument(effectiveInstr);
        if (instrObj && instrObj->GetType() == IT_SAMPLE) {
          SampleInstrument *sampleInstr =
              static_cast<SampleInstrument *>(instrObj);
          if (sampleInstr->HasSlicesForPlayback()) {
            if (sampleInstr->ShouldDisplaySliceForNote(d)) {
              showSlice = true;
              sliceIndex =
                  static_cast<uint8_t>(d - SampleInstrument::SliceNoteBase);
            } else {
              invalidSlice = true;
            }
          }
        }
      }
      if (showSlice) {
        npf_snprintf(buffer, sizeof(buffer), "SL%02u",
                     static_cast<unsigned>(sliceIndex));
      } else if (invalidSlice) {
        npf_snprintf(buffer, sizeof(buffer), "SL**");
      } else {
        note2char(d, buffer);
      }
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
        npf_snprintf(buffer, sizeof(buffer), "I%2.2x:", d);
        etl::string<32 - BATTERY_GAUGE_WIDTH> instrLine = buffer;
        setTextProps(props, 1, j, true);
        GUIPoint location = GetTitlePosition();
        location._x += 10; // make space for "Phrase %2.2x"
        InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
        I_Instrument *instr = bank->GetInstrument(d);
        instrLine += instr->GetDisplayName();
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

  for (int j = 0; j < 16; j++) {
    FourCC command = *f++;
    setTextProps(props, 2, j, false);
    DrawString(pos._x, pos._y, command.c_str(), props);
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

  for (int j = 0; j < 16; j++) {
    FourCC command = *f++;
    setTextProps(props, 4, j, false);
    DrawString(pos._x, pos._y, command.c_str(), props);
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
    cmdEditField_.SetFocus();
    cmdEditField_.Draw(w_);
  };
};

void PhraseView::OnPlayerUpdate(PlayerEventType eventType, unsigned int tick) {
  // Since this can be called from core1 via the Observer pattern,
  // we need to ensure we don't call any drawing functions directly
  // Instead of drawing directly, we'll just update our state and let
  // AnimationUpdate handle the actual drawing

  // Set the consolidated flag for UI updates
  needsUIUpdate_ = true;

  // Update the play position for use in AnimationUpdate
  Player *player = Player::GetInstance();
  if (player && player->GetSequencerMode() == SM_LIVE) {
    needsLiveIndicatorUpdate_ = true;
  }
};

void PhraseView::AnimationUpdate() {
  // First call the parent class implementation to draw the battery gauge
  ScreenView::AnimationUpdate();

  // Get player instance safely
  Player *player = Player::GetInstance();

  // Only process updates if we're fully initialized
  if (!viewData_ || !player) {
    return;
  }

  GUITextProperties props;
  // Always update VU meter even if other parts of UI dont need updating
  drawMasterVuMeter(player, props, false, 25);

  // Handle any pending updates from OnPlayerUpdate using the consolidated flag
  // This ensures all UI drawing happens on the "main" thread (core0)
  if (needsUIUpdate_) {
    // Draw notes
    drawNotes();

    // Draw play position marker
    GUIPoint anchor = GetAnchor();
    GUIPoint pos = anchor;
    pos._x -= 1;

    SetColor(CD_NORMAL);

    // Clear last played position
    pos._y = anchor._y + lastPlayingPos_;
    DrawString(pos._x, pos._y, " ", props);

    // Only update play position if player is running
    if (player->IsRunning()) {
      // Loop on all channels to see if one of them is playing current phrase
      for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
        if (player->IsChannelPlaying(i)) {
          if (viewData_->currentPlayPhrase_[i] == viewData_->currentPhrase_ &&
              viewData_->playMode_ != PM_AUDITION) {
            pos._y = anchor._y + viewData_->phrasePlayPos_[i];
            if (!player->IsChannelMuted(i)) {
              SetColor(CD_ACCENT);
              DrawString(pos._x, pos._y, ">", props);
            } else {
              SetColor(CD_ACCENTALT);
              DrawString(pos._x, pos._y, "-", props);
            }
            SetColor(CD_CURSOR);
            lastPlayingPos_ = viewData_->phrasePlayPos_[i];
            break;
          }
        }
      }
    }

    // Draw live indicators if in live mode
    if (player->GetSequencerMode() == SM_LIVE) {
      pos = anchor;
      pos._x -= 1;
      SetColor(CD_ACCENT);

      for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
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

    // Create a memory barrier to ensure proper synchronization between cores
    createMemoryBarrier();

    needsLiveIndicatorUpdate_ = false;

    // Reset the consolidated flag
    needsUIUpdate_ = false;
  }

  // Flush the window to ensure changes are displayed
  w_.Flush();
}
void PhraseView::printHelpLegend(FourCC command, GUITextProperties props) {
  if (command == FourCC::InstrumentCommandNone) {
    // no command -> no help text
    return;
  }

  char **helpLegend = getHelpLegend(command);
  char line[32]; //-1 for 1char space start of line
  // first clear top line upto battery gauge
  DrawString(0, 0, "                           ", props);
  // TODO: use ClearTextRect instead of DrawString() once it is implemented
  // ClearTextRect(0, 0, SCREEN_WIDTH - BATTERY_GAUGE_WIDTH, 0);
  strcpy(line, " ");
  strcpy(line, helpLegend[0]);
  DrawString(0, 0, line, props);
  memset(line, ' ', 32);
  if (helpLegend[1] != NULL) {
    strcpy(line, helpLegend[1]);
    DrawString(0, 1, line, props);
  }
}
