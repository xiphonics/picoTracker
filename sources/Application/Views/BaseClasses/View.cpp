/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "View.h"
#include "Application/AppWindow.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "Application/Utils/mathutils.h"
#include "Application/Views/RecordView.h"
#include "Application/Views/SampleEditorView.h"
#include "ModalView.h"
#include "System/Console/Trace.h"
#include <UIFramework/SimpleBaseClasses/EventManager.h>
#include <nanoprintf.h>

bool View::initPrivate_ = false;
ColorDefinition View::currentRectColor_ = CD_NORMAL;

int View::margin_ = 0;
int View::songRowCount_ = 16;
BatteryState View::batteryState_ = {
    .percentage = 0,
    .voltage_mv = 0,
    .temperature_c = 0,
    .charging = false,
};

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

  // Initialize VU meter tracking variables
  for (int i = 0; i < SONG_CHANNEL_COUNT + 1; i++) {
    prevLeftVU_[i] = 0;
    prevRightVU_[i] = 0;
  }
}
GUIPoint View::GetAnchor() {
  // Original code had a dynamic anchor point dending on song count, but
  // changing the song count didn't work anyway given that there are many places
  // where it was statically defined as 8. Other screens also don't fit with a
  // dynamic anchor point
  return GUIPoint(5, 3);
}

GUIPoint View::GetTitlePosition() { return GUIPoint(0, 0); };

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
  strcpy(buffer, "M TT");
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
  case VT_MIXER:
    pos._y += 3;
    DrawString(pos._x, pos._y, "M", props);
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

void View::drawMasterVuMeter(Player *player, GUITextProperties props,
                             bool forceRedraw, uint8_t xoffset) {
  stereosample playerLevel = player->GetMasterLevel();

  // Convert to dB
  int leftDb = amplitudeToDb((playerLevel >> 16) & 0xFFFF);
  int rightDb = amplitudeToDb(playerLevel & 0xFFFF);

  // Map dB to bar levels  -60dB to 0dB range mapped to 0-15 bars
  int leftBars = std::max(0, std::min(VU_METER_HEIGHT, (leftDb + 60) / 4));
  int rightBars = std::max(0, std::min(VU_METER_HEIGHT, (rightDb + 60) / 4));

  // we start at the bottom of the VU meter and draw it growing upwards
  GUIPoint pos = GetAnchor();
  pos._x += xoffset;
  pos._y += VU_METER_HEIGHT - 1; // -1 to align with song grid

  // Use index 0 for the master VU meter
  drawVUMeter(leftBars, rightBars, pos, props, 0, forceRedraw);
}

void View::drawVUMeter(uint8_t leftBars, uint8_t rightBars, GUIPoint pos,
                       GUITextProperties props, int vuIndex, bool forceRedraw) {

  // Clamp the values to the maximum height
  leftBars = std::min(leftBars, (uint8_t)VU_METER_HEIGHT);
  rightBars = std::min(rightBars, (uint8_t)VU_METER_HEIGHT);

  // Add inertia effect by limiting the rate of change
  // Maximum step change allowed per update
  const int maxStepChange = 2;

  // For rising levels (current > previous), allow faster response
  if (leftBars > prevLeftVU_[vuIndex]) {
    // If the difference is greater than maxStepChange, limit it
    if (leftBars - prevLeftVU_[vuIndex] > maxStepChange) {
      leftBars = prevLeftVU_[vuIndex] + maxStepChange;
    }
  }
  // For falling levels (current < previous), add more inertia for a slower fall
  else if (leftBars < prevLeftVU_[vuIndex]) {
    // Use a smaller step for falling levels to create more inertia
    const int fallStepChange = 1;
    if (prevLeftVU_[vuIndex] - leftBars > fallStepChange) {
      leftBars = prevLeftVU_[vuIndex] - fallStepChange;
    }
  }

  // Same for right channel
  if (rightBars > prevRightVU_[vuIndex]) {
    if (rightBars - prevRightVU_[vuIndex] > maxStepChange) {
      rightBars = prevRightVU_[vuIndex] + maxStepChange;
    }
  } else if (rightBars < prevRightVU_[vuIndex]) {
    const int fallStepChange = 1;
    if (prevRightVU_[vuIndex] - rightBars > fallStepChange) {
      rightBars = prevRightVU_[vuIndex] - fallStepChange;
    }
  }

  // Left channel: Handle level changes
  if (forceRedraw || leftBars != prevLeftVU_[vuIndex]) {
    // If forcing redraw or level changed, redraw the entire meter

    // First clear the entire meter area with inversion disabled
    props.invert_ = false;
    SetColor(CD_BACKGROUND);
    for (int i = 0; i < VU_METER_HEIGHT; i++) {
      DrawString(pos._x, pos._y - i, " ", props);
    }

    // Then draw the active cells with inversion enabled
    props.invert_ = true;
    for (int i = 0; i < leftBars; i++) {
      // Set appropriate color based on level
      if (i == VU_METER_CLIP_LEVEL) {
        SetColor(CD_ERROR);
      } else if (i > VU_METER_WARN_LEVEL) {
        SetColor(CD_WARN);
      } else {
        SetColor(CD_INFO);
      }
      DrawString(pos._x, pos._y - i, " ", props);
    }
  }
  // If not forcing redraw and leftBars == prevLeftVU_[vuIndex], do nothing for
  // left channel

  // Right channel: Handle level changes
  if (forceRedraw || rightBars != prevRightVU_[vuIndex]) {
    // If forcing redraw or level changed, redraw the entire meter

    // First clear the entire meter area with inversion disabled
    props.invert_ = false;
    SetColor(CD_BACKGROUND);
    for (int i = 0; i < VU_METER_HEIGHT; i++) {
      DrawString(pos._x + 1, pos._y - i, " ", props);
    }

    // Then draw the active cells with inversion enabled
    props.invert_ = true;
    for (int i = 0; i < rightBars; i++) {
      // Set appropriate color based on level
      if (i == VU_METER_CLIP_LEVEL) {
        SetColor(CD_ERROR);
      } else if (i > VU_METER_WARN_LEVEL) {
        SetColor(CD_WARN);
      } else {
        SetColor(CD_INFO);
      }
      DrawString(pos._x + 1, pos._y - i, " ", props);
    }
  }
  // If not forcing redraw and rightBars == prevRightVU_[vuIndex], do nothing
  // for right channel

  // Store the current values for next time
  prevLeftVU_[vuIndex] = leftBars;
  prevRightVU_[vuIndex] = rightBars;
}

void View::drawPlayTime(Player *player, GUIPoint pos,
                        GUITextProperties &props) {
  char strbuffer[10];

  SetColor(CD_NORMAL);
  props.invert_ = false;
  int time = int(player->GetPlayTime());
  int mi = time / 60;
  int se = time - mi * 60;
  npf_snprintf(strbuffer, sizeof(strbuffer), "%2.2d:%2.2d", mi, se);
  DrawString(pos._x, pos._y, strbuffer, props);
}

void View::DoModal(ModalView *view, ModalViewCallback cb) {
  modalView_ = view;
  modalView_->OnFocus();
  modalViewCallback_ = cb;
  isDirty_ = true;
};

void View::DismissModal() {
  if (modalView_ && modalView_->IsFinished()) {
    if (modalViewCallback_) {
      modalViewCallback_(*this, *modalView_);
    }
    SAFE_DELETE(modalView_);
    isDirty_ = true;
  }
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

void View::SetDirty(bool isDirty) { isDirty_ = isDirty; };

void View::ProcessButton(unsigned short mask, bool pressed) {
  isDirty_ = false;

  if (!pressed) {
    powerButtonPressed_ = false;
  } else if (mask & EPBM_POWER) {
    powerButtonPressed_ = pressed;
  }

  // Normal button processing
  if (modalView_) {
    modalView_->ProcessButton(mask, pressed);
    // checks if modal is done and if it is disposes of it:
    DismissModal();
  } else {
    ProcessButtonMask(mask, pressed);
  }

  if (isDirty_)
    ((AppWindow &)w_).SetDirty();
};

void View::Clear() { ((AppWindow &)w_).Clear(); }

void View::ForceClear() { ((AppWindow &)w_).Clear(true); }

void View::SetColor(ColorDefinition cd) { ((AppWindow &)w_).SetColor(cd); };

void View::ClearTextRect(int x, int y, int w, int h) {
  GUIRect rect(x, y, (x + w), (y + h));
  w_.ClearTextRect(rect);
};

void View::DrawString(int x, int y, const char *txt, GUITextProperties &props) {
  GUIPoint pos(x, y);
  w_.DrawString(txt, pos, props);
};

void View::DrawRect(GUIRect &r, ColorDefinition color) {
  if (View::currentRectColor_ != color) {
    View::currentRectColor_ = color;
    w_.SetCurrentRectColor(AppWindow::GetColor(color));
  }
  w_.DrawRect(r);
}

void View::drawBattery(GUITextProperties &props) {
  // only update the voltage once per second
  if (AppWindow::GetAnimationFrameCounter() % PICO_CLOCK_HZ == 0) {
    System *sys = System::GetInstance();
    sys->GetBatteryState(batteryState_);
    // Trace::Debug("Battery: %d%%", batteryState_.percentage);
  }

  GUIPoint battpos = GetAnchor();
  battpos._y = 0;
  battpos._x = 27;

  // use define to choose between drawing battery percentage or battery level as
  // "+" bars
  SetColor(CD_INFO);
  char *battText;
#if BATTERY_LEVEL_AS_PERCENTAGE
  char battTextBuffer[8];
  battText = battTextBuffer;
  if (batteryState_.charging) {
    SetColor(CD_ACCENT);
    npf_snprintf(battText, 8, "[CHG]");
  } else {
    if (batteryState_.percentage == 100) {
      npf_snprintf(battText, 8, "[FUL]");
    } else {
      npf_snprintf(battText, 8, "[%2d%%]", batteryState_.percentage);
    }
  }
#else
  if (batteryState_.charging) {
    SetColor(CD_ACCENT);
    battText = (char *)"[CHG]";
  } else if (batteryState_.percentage >= 90) {
    battText = (char *)"[+++]";
  } else if (batteryState_.percentage >= 60) {
    battText = (char *)"[++ ]";
  } else if (batteryState_.percentage >= 30) {
    SetColor(CD_WARN);
    battText = (char *)"[+  ]";
  } else {
    SetColor(CD_ERROR);
    battText = (char *)"[   ]";
  }
#endif

  DrawString(battpos._x, battpos._y, battText, props);
}

// Draw power button UI overlay
void View::drawPowerButtonUI(GUITextProperties &props) {
  // Only process and draw UI when power button is pressed
  if (powerButtonPressed_) {
    char countdownMessage[SCREEN_WIDTH];
    powerButtonHoldCount_++;

    int remainingSeconds = 3 - (powerButtonHoldCount_ / PICO_CLOCK_HZ);
    if (remainingSeconds < 0) {
      remainingSeconds = 0;
    }

    snprintf(countdownMessage, sizeof(countdownMessage),
             "Hold for shutdown (%d sec)", remainingSeconds);

    if (remainingSeconds == 0) {
      Trace::Debug("Power button held for threshold time, Powerdown!");

      System::GetInstance()->PowerDown();
    }

    // Calculate center position for the message
    GUIPoint pos = GetAnchor();
    uint16_t mesgLen = strlen(countdownMessage);
    pos._x = (SCREEN_WIDTH - mesgLen) / 2;
    pos._y = SCREEN_HEIGHT / 2 - 1;

    // Draw a background box
    SetColor(CD_BACKGROUND);
    for (int y = pos._y - 1; y <= pos._y + 1; y++) {
      for (int x = pos._x - 1; x <= (uint16_t)(pos._x + mesgLen + 1); x++) {
        GUITextProperties invProps = props;
        invProps.invert_ = true;
        DrawString(x, y, " ", invProps);
      }
    }

    // Draw the message
    SetColor(CD_EMPHASIS);
    DrawString(pos._x, pos._y, countdownMessage, props);
  } else if (powerButtonHoldCount_ > 0) {
    // Reset hold counter when button is released
    powerButtonHoldCount_ = 0;

    // Force immediate redraw by calling DrawView directly
    // This will redraw the entire screen with the correct content
    DrawView();

    Trace::Debug("Power button released! View redrawn.");
  }
}

void View::switchToRecordView() {
  // recording view only for the Advance
#ifndef ADV
  return;
#endif
  if (!Player::GetInstance()->IsRunning()) {
    RecordView::SetSourceViewType(viewType_);
    SampleEditorView::SetSourceViewType(viewType_);
    ViewType vt = VT_RECORD;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
  }
}
