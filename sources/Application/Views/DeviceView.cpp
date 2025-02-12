#include "DeviceView.h"
#include "Application/AppWindow.h"
#include "Application/Model/Scale.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Utils/char.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UITempoField.h"
#include "Services/Midi/MidiService.h"
#include "System/System/System.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include <nanoprintf.h>

#define MAX_COLOR_VALUE 0xFFFFFF

#define ACTION_BOOTSEL MAKE_FOURCC('B', 'O', 'O', 'T')

static void BootselCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    reset_usb_boot(0, 0);
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

  position._y += 1;
  v = config->FindVariable(FourCC::VarLineOut);
  intVarField_.emplace_back(position, *v, "Line Out Mode: %s", 0, 2, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

  position._y += 1;
  v = config->FindVariable(FourCC::VarRemoteUI);
  intVarField_.emplace_back(position, *v, "Remote UI: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

  position._y += 2;
  v = config->FindVariable(FourCC::VarUIFont);
  intVarField_.emplace_back(position, *v, "Font: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

  position._y += 1;
  v = config->FindVariable(FourCC::VarFGColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Foreground: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_NORMAL, position);

  position._y += 1;
  v = config->FindVariable(FourCC::VarBGColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Background: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  // dont need background color swatch as it will always be invisible against
  // background anyways

  position._y += 1;
  v = config->FindVariable(FourCC::VarHI1Color);
  bigHexVarField_.emplace_back(position, *v, 6, "HiColor1:   %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_HILITE1, position);

  position._y += 1;
  v = config->FindVariable(FourCC::VarHI2Color);
  bigHexVarField_.emplace_back(position, *v, 6, "HiColor2:   %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_HILITE2, position);

  position._y += 1;
  v = config->FindVariable(FourCC::VarConsoleColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Console:    %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_CONSOLE, position);

  position._y += 1;
  v = config->FindVariable(FourCC::VarCursorColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Cursor:     %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_CURSOR, position);

  position._y += 1;
  v = config->FindVariable(FourCC::VarInfoColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Info:       %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_INFO, position);

  position._y += 1;
  v = config->FindVariable(FourCC::VarWarnColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Warn:       %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_WARN, position);

  position._y += 1;
  v = config->FindVariable(FourCC::VarErrorColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Error:      %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_ERROR, position);

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

  FieldView::ProcessButtonMask(mask);

  if (mask & EPBM_NAV) {
    if (mask & EPBM_DOWN) {
      ViewType vt = VT_PROJECT;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
  } else {
    if (mask & EPBM_PLAY) {
      Player *player = Player::GetInstance();
      player->OnStartButton(PM_SONG, viewData_->songX_, false,
                            viewData_->songX_);
    }
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

  drawBattery(props);

  FieldView::Redraw();

  SetColor(CD_NORMAL);
  drawMap();

  pos._x = SCREEN_MAP_WIDTH + 1;
  pos._y = SCREEN_HEIGHT - 1;
  npf_snprintf(projectString, sizeof(projectString), "Build %s%s_%s",
               PROJECT_NUMBER, PROJECT_RELEASE, BUILD_COUNT);
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
  case FourCC::VarFGColor:
  case FourCC::VarBGColor:
  case FourCC::VarHI1Color:
  case FourCC::VarHI2Color:
  case FourCC::VarConsoleColor:
  case FourCC::VarCursorColor:
  case FourCC::VarInfoColor:
  case FourCC::VarWarnColor:
  case FourCC::VarErrorColor:
    Trace::Log("DEVICE", "Color updated!");
    ((AppWindow &)w_).UpdateColorsFromConfig();
    ((AppWindow &)w_).Clear(true);
    w_.Update(true);
    break;

  case FourCC::VarLineOut: {
    MessageBox *mb =
        new MessageBox(*this, "Reboot for new Audio Level!", MBBF_OK);
    DoModal(mb);
  }

  default:
    NInvalid;
    break;
  };
  focus->Draw(w_);
  isDirty_ = true;

  Trace::Log("DEVICEVIEW", "persist config changes!");
  Config *config = Config::GetInstance();
  config->Save();
};

void DeviceView::addSwatchField(ColorDefinition color, GUIPoint position) {
  position._x -= 5;
  swatchField_.emplace_back(position, color);
  fieldList_.insert(fieldList_.end(), &(*swatchField_.rbegin()));
}

void DeviceView::AnimationUpdate() {
  // redraw batt gauge on every clock tick (~1Hz) even when not playing
  // and not redrawing due to user cursor navigation
  GUITextProperties props;
  drawBattery(props);
  w_.Flush();
};