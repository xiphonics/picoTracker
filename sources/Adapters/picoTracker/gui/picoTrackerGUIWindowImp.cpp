#include "picoTrackerGUIWindowImp.h"
#include "Application/Commands/EventDispatcher.h"
#include "Application/Model/Config.h"
#include "Application/Utils/char.h"
#include "System/Console/Trace.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include "UIFramework/BasicDatas/GUIPoint.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#ifdef USB_REMOTE_UI
#include "picoRemoteUI.h"
#endif
#include <string>

#define to_rgb565(color)                                                       \
  ((color._r & 0b11111000) << 8) | ((color._g & 0b11111100) << 3) |            \
      (color._b >> 3)

// classic picotracker mapping
static GUIEventPadButtonType eventMappingPico[10] = {
    EPBT_LEFT,  // SW1
    EPBT_DOWN,  // SW2
    EPBT_RIGHT, // SW3
    EPBT_UP,    // SW4
    EPBT_L,     // SW5
    EPBT_B,     // SW6
    EPBT_A,     // SW7
    EPBT_R,     // SW8
    EPBT_START, // SW9
    EPBT_SELECT // No SW
};

static GUIEventPadButtonType *eventMapping = eventMappingPico;

// Initialize static members
picoTrackerGUIWindowImp *picoTrackerGUIWindowImp::instance_ = NULL;
uint32_t picoTrackerGUIWindowImp::tickCounter_ = 0;

picoTrackerGUIWindowImp::picoTrackerGUIWindowImp(GUICreateWindowParams &p) {
  mode0_init();
  instance_ = this;

  Config *config = Config::GetInstance();

  auto remoteUIVar =
      (WatchedVariable *)config->FindVariable(FourCC::VarRemoteUI);

  // register to receive updates to remoteui setting
  remoteUIVar->AddObserver(*this);
  auto remoteui = remoteUIVar->GetInt();
  remoteUIEnabled_ = remoteui != 0;

  auto uiFontVar = (WatchedVariable *)config->FindVariable(FourCC::VarUIFont);
  // register to receive updates to remoteui setting
  uiFontVar->AddObserver(*this);
  auto uifontIndex = uiFontVar->GetInt();
  mode0_set_font_index(uifontIndex);
#ifdef USB_REMOTE_UI
  if (remoteUIEnabled_) {
    SendFont(uifontIndex);
  }
#endif
};

picoTrackerGUIWindowImp::~picoTrackerGUIWindowImp() {}

#ifdef USB_REMOTE_UI
void picoTrackerGUIWindowImp::SendFont(uint8_t uifontIndex) {
  char remoteUIBuffer[3];
  remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
  remoteUIBuffer[1] = SETFONT_CMD;
  remoteUIBuffer[2] = uifontIndex + ASCII_SPACE_OFFSET;
  sendToUSBCDC(remoteUIBuffer, 3);
}
#endif

void picoTrackerGUIWindowImp::DrawChar(const char c, GUIPoint &pos,
                                       GUITextProperties &p) {
  //  Trace::Debug("Draw char \"%c\" at pos x:%ld (%ld), y:%ld (%ld) - invert:
  //  %d", c, pos._x, pos._x / 8, pos._y, pos._y / 8, p.invert_);

  uint8_t x = pos._x / 8;
  uint8_t y = pos._y / 8;
  mode0_set_cursor(x, y);
  mode0_putc(c, p.invert_);
#ifdef USB_REMOTE_UI
  if (remoteUIEnabled_) {
    char remoteUIBuffer[6];
    remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
    remoteUIBuffer[1] = TEXT_CMD;
    remoteUIBuffer[2] = c;
    remoteUIBuffer[3] = x + ASCII_SPACE_OFFSET; // to avoid sending NUL (aka 0)
    remoteUIBuffer[4] = y + ASCII_SPACE_OFFSET;
    remoteUIBuffer[5] = p.invert_ ? 127 : 0;
    sendToUSBCDC(remoteUIBuffer, 6);
  }
#endif
}

void picoTrackerGUIWindowImp::DrawRect(GUIRect &r) {
  Trace::Debug("GUI DrawRect call");
};

void picoTrackerGUIWindowImp::Clear(GUIColor &c, bool overlay) {
  mode0_color_t backgroundColor = GetColor(c);
  mode0_set_background(backgroundColor);
  mode0_clear(backgroundColor);
#ifdef USB_REMOTE_UI
  if (remoteUIEnabled_) {
    char remoteUIBuffer[5];
    remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
    remoteUIBuffer[1] = CLEAR_CMD;
    remoteUIBuffer[2] = c._r;
    remoteUIBuffer[3] = c._g;
    remoteUIBuffer[4] = c._b;
    sendToUSBCDC(remoteUIBuffer, 5);
  }
#endif
};

void picoTrackerGUIWindowImp::ClearRect(GUIRect &r) {
  Trace::Debug("GUI ClearRect call");
};

// Keep track of the last RGB values set for each palette index
static uint16_t lastPaletteRGB[16] = {0};

mode0_color_t picoTrackerGUIWindowImp::GetColor(GUIColor &c) {
  // Palette index should always be < 16
  if (c._paletteIndex >= 16) {
    return MODE0_NORMAL; // Default to normal color if index is invalid
  }

  // Convert the color to RGB565 format
  uint16_t rgb565 = to_rgb565(c);

  // Only update the palette if the color has changed
  if (lastPaletteRGB[c._paletteIndex] != rgb565) {
    mode0_set_palette_color(c._paletteIndex, rgb565);
    lastPaletteRGB[c._paletteIndex] = rgb565;
  }

  return (mode0_color_t)c._paletteIndex;
}

void picoTrackerGUIWindowImp::SetColor(GUIColor &c) {
  mode0_color_t color = GetColor(c);
  mode0_set_foreground(color);
#ifdef USB_REMOTE_UI
  if (remoteUIEnabled_) {
    char remoteUIBuffer[5];
    remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
    remoteUIBuffer[1] = SETCOLOR_CMD;
    remoteUIBuffer[2] = c._r;
    remoteUIBuffer[3] = c._g;
    remoteUIBuffer[4] = c._b;
    sendToUSBCDC(remoteUIBuffer, 5);
  }
#endif
};

void picoTrackerGUIWindowImp::Lock(){};

void picoTrackerGUIWindowImp::Unlock(){};

void picoTrackerGUIWindowImp::Flush() { mode0_draw_changed(); };

void picoTrackerGUIWindowImp::Invalidate() {
  picoTrackerEventQueue::GetInstance()->push(picoTrackerEvent(PICO_FLUSH));
};

void picoTrackerGUIWindowImp::PushEvent(GUIEvent &event) {
  Trace::Debug("GUI PushEvent");
};

GUIRect picoTrackerGUIWindowImp::GetRect() {
  Trace::Debug("GUI GetRect");
  return GUIRect(0, 0, 320, 240);
}

void picoTrackerGUIWindowImp::ProcessEvent(picoTrackerEvent &event) {
  switch (event.type_) {
  case PICO_REDRAW:
    instance_->_window->Update(true);
#ifdef USB_REMOTE_UI
    // send font update
    if (instance_->remoteUIEnabled_) {
      Config *config = Config::GetInstance();
      auto uiFontVar = config->FindVariable(FourCC::VarUIFont);
      int uifontIndex = uiFontVar->GetInt();
      instance_->SendFont(uifontIndex);
    }
#endif
    break;
  case PICO_FLUSH:
    instance_->_window->Update(false);
    break;
  case PICO_CLOCK:
    instance_->_window->ClockTick(instance_->tickCounter_++);
    break;
  case LAST:
    break;
  }
}

void picoTrackerGUIWindowImp::ProcessButtonChange(uint16_t changeMask,
                                                  uint16_t buttonMask) {
  int e = 1;
  System *system = System::GetInstance();
  unsigned long now = system->GetClock();
  for (int i = 0; i < 10; i++) {
    if (changeMask & e) {
      GUIEventType type = (buttonMask & e) ? ET_PADBUTTONDOWN : ET_PADBUTTONUP;

      GUIEvent event(eventMapping[i], type, now, 0, 0, 0);
      instance_->_window->DispatchEvent(event);
    }
    e = e << 1;
  }
}

void picoTrackerGUIWindowImp::Update(Observable &o, I_ObservableData *d) {
  WatchedVariable &v = (WatchedVariable &)o;
  switch (v.GetID()) {
  case FourCC::VarRemoteUI: {
    auto remoteui = v.GetInt();
    remoteUIEnabled_ = remoteui != 0;
  } break;
  case FourCC::VarUIFont: {
    auto uifont = v.GetInt();
    mode0_set_font_index(uifont);
#ifdef USB_REMOTE_UI
    if (remoteUIEnabled_) {
      SendFont(uifont);
    }
#endif
  } break;
  }
}