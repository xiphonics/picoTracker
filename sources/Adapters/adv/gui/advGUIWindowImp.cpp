/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advGUIWindowImp.h"
#include "Adapters/adv/filesystem/advFileSystem.h"
#include "Adapters/adv/utils/utils.h"
#include "Application/Model/Config.h"
#include "Application/Utils/char.h"
#include "Player/Player.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "advRemoteUI.h"
#include <string.h>
#include <string>

#define to_rgb565(color)                                                       \
  ((color._r & 0b11111000) << 8) | ((color._g & 0b11111100) << 3) |            \
      (color._b >> 3)

// classic picotracker mapping
static GUIEventPadButtonType eventMappingPico[11] = {
    EPBT_LEFT,   // SW1
    EPBT_DOWN,   // SW2
    EPBT_RIGHT,  // SW3
    EPBT_UP,     // SW4
    EPBT_L,      // SW5
    EPBT_B,      // SW6
    EPBT_A,      // SW7
    EPBT_R,      // SW8
    EPBT_START,  // SW9
    EPBT_SELECT, // No SW
    EPBT_POWER   // Power button
};

advGUIWindowImp *instance_;

advGUIWindowImp::advGUIWindowImp(GUICreateWindowParams &p) {
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
  display_set_font_index(uifontIndex);
  if (remoteUIEnabled_) {
    SendFont(uifontIndex);
  }
};

advGUIWindowImp::~advGUIWindowImp() {}

void advGUIWindowImp::SendFont(uint8_t uifontIndex) {
  char remoteUIBuffer[3];
  remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
  remoteUIBuffer[1] = SETFONT_CMD;
  remoteUIBuffer[2] = uifontIndex + ASCII_SPACE_OFFSET;
  sendToUSBCDCBuffered(remoteUIBuffer, 3);
}

void advGUIWindowImp::DrawChar(const char c, GUIPoint &pos,
                               GUITextProperties &p) {
  //  Trace::Debug("Draw char \"%c\" at pos x:%ld (%ld), y:%ld (%ld) - invert:
  //  %d", c, pos._x, pos._x / 8, pos._y, pos._y / 8, p.invert_);

  uint8_t x = pos._x / 8;
  uint8_t y = pos._y / 8;
  display_set_cursor(x, y);
  display_putc(c, p.invert_);
  if (remoteUIEnabled_) {
    char remoteUIBuffer[6];
    remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
    remoteUIBuffer[1] = TEXT_CMD;
    remoteUIBuffer[2] = c;
    remoteUIBuffer[3] = x + ASCII_SPACE_OFFSET; // to avoid sending NUL (aka 0)
    remoteUIBuffer[4] = y + ASCII_SPACE_OFFSET;
    remoteUIBuffer[5] = p.invert_ ? 127 : 0;
    sendToUSBCDCBuffered(remoteUIBuffer, 6); // Use the buffered function
  }
}

void advGUIWindowImp::DrawString(const char *string, GUIPoint &pos,
                                 GUITextProperties &p, bool overlay) {
  Trace::Debug("draw string");
  display_set_cursor(pos._x, pos._y);
  display_print(string, p.invert_);
};

void advGUIWindowImp::DrawRect(GUIRect &r) {
  Trace::Debug("GUI DrawRect call");
};

void advGUIWindowImp::DrawRect(uint8_t colorIdx, GUIRect &r) {
  display_fill_rect(colorIdx, r.Left(), r.Top(), r.Width(), r.Height());
  if (remoteUIEnabled_) {
    char remoteUIBuffer[7];
    remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
    remoteUIBuffer[1] = DRAWRECT_CMD;
    remoteUIBuffer[2] = colorIdx;
    remoteUIBuffer[3] = r.Left();
    remoteUIBuffer[4] = r.Top();
    remoteUIBuffer[5] = r.Width();
    remoteUIBuffer[6] = r.Height();
    sendToUSBCDC(remoteUIBuffer, 7);
  }
}

void advGUIWindowImp::Clear(GUIColor &c, bool overlay) {
  color_t backgroundColor = GetColor(c);
  display_set_background(backgroundColor);
  display_clear(backgroundColor);
  if (remoteUIEnabled_) {
    char remoteUIBuffer[5];
    remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
    remoteUIBuffer[1] = CLEAR_CMD;
    remoteUIBuffer[2] = c._r;
    remoteUIBuffer[3] = c._g;
    remoteUIBuffer[4] = c._b;
    sendToUSBCDCBuffered(remoteUIBuffer, 5); // Use the buffered function
  }
};

void advGUIWindowImp::ClearRect(GUIRect &r) {
  Trace::Debug("GUI ClearRect call");
};

color_t advGUIWindowImp::GetColor(GUIColor &c) {
  // Palette index should always be < 16. Wont check it.
  // TODO: should not be redefining the palette colors every call
  display_set_palette_color(c._paletteIndex, to_rgb565(c));
  return (color_t)c._paletteIndex;
}

void advGUIWindowImp::SetColor(GUIColor &c) {
  color_t color = GetColor(c);
  display_set_foreground(color);
  if (remoteUIEnabled_) {
    char remoteUIBuffer[5];
    remoteUIBuffer[0] = REMOTE_UI_CMD_MARKER;
    remoteUIBuffer[1] = SETCOLOR_CMD;
    remoteUIBuffer[2] = c._r;
    remoteUIBuffer[3] = c._g;
    remoteUIBuffer[4] = c._b;
    sendToUSBCDCBuffered(remoteUIBuffer, 5); // Use the buffered function
  }
};

void advGUIWindowImp::Lock(){};

void advGUIWindowImp::Unlock(){};

void advGUIWindowImp::Flush() {
  display_draw_changed();
  // send buffered UI draw cmds out over USB
  if (remoteUIEnabled_) {
    flushRemoteUIBuffer();
  }
};

void advGUIWindowImp::Invalidate() {
  Event ev(FLUSH);
  xQueueSend(eventQueue, &ev, 0);
};

void advGUIWindowImp::PushEvent(GUIEvent &event) {
  Trace::Debug("GUI PushEvent");
};

GUIRect advGUIWindowImp::GetRect() {
  Trace::Debug("GUI GetRect");
  return GUIRect(0, 0, 320, 240);
}

void advGUIWindowImp::ProcessEvent(Event &event) {
  switch (event.type_) {
  case REDRAW:
    instance_->_window->Update(true);
    // send font update
    if (instance_->remoteUIEnabled_) {
      Config *config = Config::GetInstance();
      auto uiFontVar = config->FindVariable(FourCC::VarUIFont);
      int uifontIndex = uiFontVar->GetInt();
      instance_->SendFont(uifontIndex);
    }
    break;
  case FLUSH:
    instance_->_window->Update(false);
    break;
  case CLOCK:
    instance_->_window->ClockTick();
    break;
  case SD_DET:
    // SD reinit
    FATFS_UnLinkDriver(SDPath);
    HAL_SD_DeInit(&hsd1);
    __HAL_RCC_SDMMC1_FORCE_RESET();
    __HAL_RCC_SDMMC1_RELEASE_RESET();
    MX_SDMMC1_SD_Init();
    FATFS_LinkDriver(&SD_DMA_Driver, SDPath);
  case LAST:
    break;
  }
}

void advGUIWindowImp::ProcessButtonChange(uint16_t changeMask,
                                          uint16_t buttonMask) {
  int e = 1;
  System *system = System::GetInstance();
  unsigned long now = system->GetClock();
  for (uint32_t i = 0; i < sizeof(eventMappingPico); i++) {
    if (changeMask & e) {
      GUIEventType type = (buttonMask & e) ? ET_PADBUTTONDOWN : ET_PADBUTTONUP;

      GUIEvent event(eventMappingPico[i], type, now, 0, 0, 0);
      instance_->_window->DispatchEvent(event);
    }
    e = e << 1;
  }
}

void advGUIWindowImp::Update(Observable &o, I_ObservableData *d) {
  WatchedVariable &v = (WatchedVariable &)o;
  switch (v.GetID()) {
  case FourCC::VarRemoteUI: {
    auto remoteui = v.GetInt();
    remoteUIEnabled_ = remoteui != 0;
  } break;
  case FourCC::VarUIFont: {
    auto uifont = v.GetInt();
    display_set_font_index(uifont);
    if (remoteUIEnabled_) {
      SendFont(uifont);
    }
  } break;
  }
}

// Buffer draw commands to be sent to remote UI over USB
void advGUIWindowImp::sendToUSBCDCBuffered(const char *buf, uint32_t len) {
  if (!remoteUIEnabled_ || len == 0)
    return;

  // If the new data won't fit, flush the buffer first to make space.
  if ((remoteUIBufferPos_ + len) > sizeof(remoteUIDrawBuffer_)) {
    flushRemoteUIBuffer();
  }

  // Copy data into the buffer
  memcpy(remoteUIDrawBuffer_ + remoteUIBufferPos_, buf, len);
  remoteUIBufferPos_ += len;
}

void advGUIWindowImp::flushRemoteUIBuffer() {
  if (remoteUIBufferPos_ > 0) {
    // This is the function you were already using, which calls tud_cdc_write,
    // etc.
    sendToUSBCDC(remoteUIDrawBuffer_, remoteUIBufferPos_);
    remoteUIBufferPos_ = 0; // Reset the buffer position
  }
}
