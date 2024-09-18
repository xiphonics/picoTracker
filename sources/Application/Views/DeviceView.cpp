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

  position._y += 1;
  Variable *v = config->FindVariable(VAR_FG_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "Foreground: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_NORMAL, position);

  position._y += 1;
  v = config->FindVariable(VAR_BG_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "Background: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_BACKGROUND, position);

  position._y += 1;
  v = config->FindVariable(VAR_HI1_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "HiColor1:   %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_HILITE1, position);

  position._y += 1;
  v = config->FindVariable(VAR_HI2_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "HiColor2:   %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_HILITE2, position);

  position._y += 1;
  v = config->FindVariable(VAR_CONSOLE_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "Console:    %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_CONSOLE, position);

  position._y += 1;
  v = config->FindVariable(VAR_CURSOR_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "Cursor:     %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_CURSOR, position);

  position._y += 1;
  v = config->FindVariable(VAR_INFO_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "Info:       %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_INFO, position);

  position._y += 1;
  v = config->FindVariable(VAR_WARN_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "Warn:       %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_WARN, position);

  position._y += 1;
  v = config->FindVariable(VAR_ERROR_COLOR);
  bigHexVarField_.emplace_back(position, *v, 6, "Error:      %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_ERROR, position);

  v = config->FindVariable(VAR_MIDI_DEVICE);
  position._y += 2;
  intVarField_.emplace_back(position, *v, "MIDI device: %s", 0, 0, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  v = config->FindVariable(VAR_MIDI_SYNC);
  position._y += 1;
  intVarField_.emplace_back(position, *v, "MIDI sync: %s", 0, 0, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  v = config->FindVariable(VAR_LINEOUT);
  intVarField_.emplace_back(position, *v, "Line Out Mode: %s", 0, 2, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));

  position._y += 2;
  actionField_.emplace_back("Update firmware", ACTION_BOOTSEL, position);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);
}

DeviceView::~DeviceView() {}

void DeviceView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed)
    return;

  FieldView::ProcessButtonMask(mask);

  if (mask & EPBM_R) {
    if (mask & EPBM_DOWN) {
      ViewType vt = VT_PROJECT;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
  } else {
    if (mask & EPBM_START) {
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
  sprintf(projectString, "Device - Build %s%s_%s", PROJECT_NUMBER,
          PROJECT_RELEASE, BUILD_COUNT);

  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, projectString, props);

  FieldView::Redraw();
  drawMap();
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
  case ACTION_BOOTSEL: {
    if (!player->IsRunning()) {
      MessageBox *mb =
          new MessageBox(*this, "Reboot and lose changes?", MBBF_YES | MBBF_NO);
      DoModal(mb, BootselCallback);
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
    break;
  }
  case VAR_FG_COLOR:
  case VAR_BG_COLOR:
  case VAR_HI1_COLOR:
  case VAR_HI2_COLOR:
  case VAR_CONSOLE_COLOR:
  case VAR_CURSOR_COLOR:
  case VAR_INFO_COLOR:
  case VAR_WARN_COLOR:
  case VAR_ERROR_COLOR:
    printf("Color updated!");
    ((AppWindow &)w_).UpdateColorsFromConfig();
    ((AppWindow &)w_).Clear(true);
    w_.Update();
    break;

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
