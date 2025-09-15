/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "picoTrackerGUIWindowImp.h"
#include "Application/Commands/EventDispatcher.h"
#include "Application/Model/Config.h"
#include "Application/Utils/char.h"
#include "System/Console/Trace.h"
#include "UIFramework/BasicDatas/GUIPoint.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
#include "pico/stdlib.h"
#include "picoRemoteUI.h"
#include <stdio.h>
#include <string.h>
#include <string>

// Keep track of the last RGB values set for each palette index
static uint16_t lastPaletteRGB[16] = {0};

// Keep track of the last "SetColor()" call to track the current color palette
// index, used by DrawRect() to know which color to use when drawing to the
// devices LCD
static uint8_t lastRemoteColorIdx = 255;

static GUIEventPadButtonType *eventMapping = eventMappingPico;

// Initialize static members
picoTrackerGUIWindowImp *picoTrackerGUIWindowImp::instance_ = NULL;

picoTrackerGUIWindowImp::picoTrackerGUIWindowImp(GUICreateWindowParams &p) {
  chargfx_init();
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
  chargfx_set_font_index(uifontIndex);
  if (remoteUIEnabled_) {
    SendFont(uifontIndex);
  }
};

picoTrackerGUIWindowImp::~picoTrackerGUIWindowImp() {}

void picoTrackerGUIWindowImp::SendFont(uint8_t uifontIndex) {
  char remoteUIBuffer[3];
  remoteUIFontCommand(uifontIndex, remoteUIBuffer);
  sendToUSBCDC(remoteUIBuffer, 3);
}

void picoTrackerGUIWindowImp::DrawChar(const char c, GUIPoint &pos,
                                       GUITextProperties &p) {
  //  Trace::Debug("Draw char \"%c\" at pos x:%ld (%ld), y:%ld (%ld) - invert:
  //  %d", c, pos._x, pos._x / 8, pos._y, pos._y / 8, p.invert_);

  uint8_t x = pos._x / 8;
  uint8_t y = pos._y / 8;
  chargfx_set_cursor(x, y);
  chargfx_putc(c, p.invert_);
  if (remoteUIEnabled_) {
    char remoteUIBuffer[6];
    remoteUIDrawCharCommand(c, x, y, p.invert_, remoteUIBuffer);
    sendToUSBCDC(remoteUIBuffer, 6);
  }
}

void picoTrackerGUIWindowImp::DrawRect(GUIRect &r) {
  // This is the local drawing command for the device's own screen.
  chargfx_fill_rect(lastRemoteColorIdx, r.Left(), r.Top(), r.Width(),
                    r.Height());
  if (remoteUIEnabled_) {
    // Now, send the DrawRect command with full byte-escaping.
    // Worst-case buffer: 2 (header) + 9 payload bytes * 2 (if all are escaped)
    // = 20  bytes.
    char remoteUIBuffer[20];
    auto bufferIndex = remoteUIDrawRectCommand(r.Left(), r.Top(), r.Width(),
                                               r.Height(), remoteUIBuffer);
    sendToUSBCDC(remoteUIBuffer, bufferIndex);
  }
};

void picoTrackerGUIWindowImp::Clear(GUIColor &c, bool overlay) {
  chargfx_color_t backgroundColor = GetColor(c);
  chargfx_set_background(backgroundColor);
  chargfx_clear(backgroundColor);
  if (remoteUIEnabled_) {
    char remoteUIBuffer[5];
    remoteUIClearCommand(c._r, c._g, c._b, remoteUIBuffer);
    // log sent buffer values
    Trace::Debug("sent clear command: %d,%d,%d", c._r, c._g, c._b);
    sendToUSBCDC(remoteUIBuffer, 5);
  }
};

void picoTrackerGUIWindowImp::ClearTextRect(GUIRect &r) {
  Trace::Debug("GUI ClearTextRect call");
};

chargfx_color_t picoTrackerGUIWindowImp::GetColor(GUIColor &c) {
  // Palette index should always be < 16
  if (c._paletteIndex >= 16) {
    return CHARGFX_NORMAL; // Default to normal color if index is invalid
  }

  // Convert the color to RGB565 format
  uint16_t rgb565 = to_rgb565(c);

  // Only update the palette if the color has changed
  if (lastPaletteRGB[c._paletteIndex] != rgb565) {
    chargfx_set_palette_color(c._paletteIndex, rgb565);
    lastPaletteRGB[c._paletteIndex] = rgb565;
  }

  return (chargfx_color_t)c._paletteIndex;
}

void picoTrackerGUIWindowImp::SetColor(GUIColor &c) {
  chargfx_color_t color = GetColor(c);
  lastRemoteColorIdx = color;

  NAssert(c._r < 255);
  NAssert(c._g < 255);
  NAssert(c._b < 255);
  chargfx_set_foreground(color);
  if (remoteUIEnabled_) {
    // Buffer must be large enough for the worst case where all 3 color bytes
    // are escaped. Header (2) + 3 color components * 2 bytes/escaped_component
    // = 8 bytes.
    char remoteUIBuffer[8];
    auto bufferIndex =
        remoteUISetColorCommand(c._r, c._g, c._b, remoteUIBuffer);
    sendToUSBCDC(remoteUIBuffer, bufferIndex);
    // Trace::Debug("sent set color: %d,%d,%d", c._r, c._g, c._b);
  }
};

void picoTrackerGUIWindowImp::Lock(){};

void picoTrackerGUIWindowImp::Unlock(){};

void picoTrackerGUIWindowImp::Flush() { chargfx_draw_changed(); };

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
    // send font update
    if (instance_->remoteUIEnabled_) {
      Config *config = Config::GetInstance();
      auto uiFontVar = config->FindVariable(FourCC::VarUIFont);
      int uifontIndex = uiFontVar->GetInt();
      instance_->SendFont(uifontIndex);
    }
    break;
  case PICO_FLUSH:
    instance_->_window->Update(false);
    break;
  case PICO_CLOCK:
    instance_->_window->ClockTick();
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
    chargfx_set_font_index(uifont);
    if (remoteUIEnabled_) {
      SendFont(uifont);
    }
  } break;
  }
}