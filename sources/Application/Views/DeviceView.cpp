/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "DeviceView.h"
#include "Application/AppWindow.h"
#include "Application/Model/Scale.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Utils/char.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/SampleEditorView.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UITempoField.h"
#include "Services/Audio/Audio.h"
#include "Services/Midi/MidiService.h"
#include "System/System/System.h"
#include "platform.h"
#include <nanoprintf.h>

#define ACTION_BOOTSEL MAKE_FOURCC('B', 'O', 'O', 'T')

static void BootselCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    System *sys = System::GetInstance();
    sys->SystemBootloader();
  }
};

DeviceView::DeviceView(GUIWindow &w, ViewData *data) : FieldView(w, data) {

  GUIPoint position = GetAnchor();

  auto config = Config::GetInstance();

  Variable *v;

  v = config->FindVariable(FourCC::VarMidiDevice);
  intVarField_.emplace_back(position, *v, "MIDI device: %s", 0, 3, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

  position._y += 1;
  v = config->FindVariable(FourCC::VarMidiSync);
  // just hardcode max of 1, as only settings are "off" & "send"
  intVarField_.emplace_back(position, *v, "MIDI sync: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

#ifndef ADV
  position._y += 1;
  v = config->FindVariable(FourCC::VarLineOut);
  intVarField_.emplace_back(position, *v, "Line Out Mode: %s", 0, 2, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);
#endif

  position._y += 1;
  v = config->FindVariable(FourCC::VarRemoteUI);
  intVarField_.emplace_back(position, *v, "Remote UI: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

  position._y += 1;
  v = config->FindVariable(FourCC::VarBacklightLevel);
  // MIN brightness is 0xF (15)
  intVarField_.emplace_back(position, *v, "Display brightness: %2.2x", 0xF,
                            0xFF, 1, 16);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

#ifdef ADV
  position._y += 1;
  v = config->FindVariable(FourCC::VarOutputVolume);
  intVarField_.emplace_back(position, *v, "Output volume: %3d", 0, 100, 1, 5);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);
#endif

  position._y += 2;
  actionField_.emplace_back("Theme settings", FourCC::ActionShowTheme,
                            position);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);

  position._y += 2;
  actionField_.emplace_back("Update firmware", FourCC::ActionBootSelect,
                            position);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);
}

DeviceView::~DeviceView() {}

void DeviceView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed)
    return;

  FieldView::ProcessButtonMask(mask, pressed);

  if (mask & EPBM_EDIT) {
    if (mask & EPBM_PLAY) {
      // recording screen
      if (!Player::GetInstance()->IsRunning()) {
        switchToRecordView();
      }
    }
    if (mask & EPBM_ENTER) {
      UIIntVarField *field = (UIIntVarField *)GetFocus();
      Variable &var = field->GetVariable();
      if (field->GetVariableID() != FourCC::Default) {
        field->ProcessReset();
      }
    }
  } else if (mask & EPBM_NAV) {
    if (mask & EPBM_DOWN) {
      ViewType vt = VT_PROJECT;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
  } else if (mask & EPBM_PLAY) {
    Player *player = Player::GetInstance();
    player->OnStartButton(PM_SONG, viewData_->songX_, false, viewData_->songX_);
  };
};

void DeviceView::DrawView() {

  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title
  char projectString[SCREEN_WIDTH];
  strcpy(projectString, "Device");

  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, projectString, props);

  FieldView::Redraw();

#ifdef ADV
  // Draw battery health
  pos._x = SCREEN_MAP_WIDTH + 1;
  pos._y = 21;
  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, "Battery health:", props);

  pos._x += strlen("Battery health") + 1;
  int16_t soh = battery_health();
  char sohText[5];
  (soh < 0) ? npf_snprintf(sohText, sizeof(sohText), "%s", "NA")
            : npf_snprintf(sohText, sizeof(sohText), "%i%%", soh);
  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, sohText, props);
#endif

  SetColor(CD_NORMAL);
  drawMap();

  pos._x = SCREEN_MAP_WIDTH + 1;
  pos._y = SCREEN_HEIGHT - 1;

  npf_snprintf(projectString, sizeof(projectString), "Build %s%s_%s",
               PROJECT_NUMBER, PROJECT_RELEASE, BUILD_COUNT);
  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, projectString, props);
};

void DeviceView::Update(Observable &, I_ObservableData *data) {
  if (!hasFocus_) {
    return;
  }

  uintptr_t fourcc = (uintptr_t)data;

  UIField *focus = GetFocus();
  focus->ClearFocus();
  focus->Draw(w_);
  w_.Flush();
  focus->SetFocus();

  // Handle brightness changes directly
  if (fourcc == FourCC::VarBacklightLevel) {
    Config *config = Config::GetInstance();
    Variable *v = config->FindVariable(FourCC::VarBacklightLevel);
    if (v) {
      unsigned char brightness = (unsigned char)v->GetInt();
      System::GetInstance()->SetDisplayBrightness(brightness);
    }
    configDirty_ = true;
  }

  Player *player = Player::GetInstance();

  switch (fourcc) {
  case FourCC::ActionBootSelect: {
    if (!player->IsRunning()) {
      MessageBox *mb =
          new MessageBox(*this, "Reboot and lose changes?", MBBF_YES | MBBF_NO);
      DoModal(mb, BootselCallback);
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
    return;
  }
  case FourCC::ActionShowTheme: {
    ViewType vt = VT_THEME;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
    return;
  }
  case FourCC::VarLineOut: {
    MessageBox *mb =
        new MessageBox(*this, "Reboot for new Audio Level!", MBBF_OK);
    DoModal(mb);
    configDirty_ = true;
    break;
  }
  case FourCC::VarMidiDevice:
  case FourCC::VarMidiSync:
  case FourCC::VarRemoteUI: {
    configDirty_ = true;
    break;
  }
  case FourCC::VarOutputVolume: {
    Config *config = Config::GetInstance();
    Variable *v = config->FindVariable(FourCC::VarOutputVolume);
    if (v) {
      Audio *audio = Audio::GetInstance();
      if (audio) {
        // This unfortunate name may get confused with the actual audio pipeline
        // mixer. It's not, this sets the driver output volume
        audio->SetMixerVolume(v->GetInt());
      }
    }
    configDirty_ = true;
    break;
  }
  default:
    NInvalid;
    break;
  };
  focus->Draw(w_);
  isDirty_ = true;
};

void DeviceView::addSwatchField(ColorDefinition color, GUIPoint position) {
  position._x -= 5;
  swatchField_.emplace_back(position, color);
  fieldList_.insert(fieldList_.end(), &(*swatchField_.rbegin()));
}

void DeviceView::OnFocusLost() {
  if (configDirty_) {
    Config *config = Config::GetInstance();
    if (!config->Save()) {
      Trace::Error("DEVICEVIEW", "Failed to save device config on focus lost");
      return;
    }
    Trace::Log("DEVICEVIEW", "Saved device config on focus lost");
    configDirty_ = false;
  }
}
