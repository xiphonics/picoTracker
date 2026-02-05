/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advGUIWindowImp.h"
#include "Adapters/adv/audio/record.h"
#include "Adapters/adv/filesystem/advFileSystem.h"
#include "Adapters/adv/utils/utils.h"
#include "Application/Model/Config.h"
#include "Application/Utils/char.h"
#include "Player/Player.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "advRemoteUI.h"
#include <string.h>
#include <string>

// Keep track of the last "SetColor()" call to track the current color palette
// index, used by DrawRect() to know which color to use when drawing to the
// devices LCD
static uint8_t lastRemoteColorIdx = 255;

void appwindow_set_sdcard_present(bool present);

advGUIWindowImp *instance_;

advGUIWindowImp::advGUIWindowImp(GUICreateWindowParams &p) {

  // stash ref to singleton for use in static methods
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
  remoteUIFontCommand(uifontIndex, remoteUIBuffer);
  sendToUSBCDCBuffered(remoteUIBuffer, 3);
}

void advGUIWindowImp::DrawChar(const char c, const GUIPoint &pos,
                               const GUITextProperties &p) {
  //  Trace::Debug("Draw char \"%c\" at pos x:%ld (%ld), y:%ld (%ld) - invert:
  //  %d", c, pos._x, pos._x / 8, pos._y, pos._y / 8, p.invert_);

  uint8_t x = pos._x / 8;
  uint8_t y = pos._y / 8;
  display_set_cursor(x, y);
  display_putc(c, p.invert_);
  if (remoteUIEnabled_) {
    char remoteUIBuffer[6];
    remoteUIDrawCharCommand(c, x, y, p.invert_, remoteUIBuffer);
    sendToUSBCDCBuffered(remoteUIBuffer, 6); // Use the buffered function
  }
}

void advGUIWindowImp::DrawString(const char *string, const GUIPoint &pos,
                                 GUITextProperties &p, bool overlay) {
  Trace::Debug("draw string");
  display_set_cursor(pos._x, pos._y);
  display_print(string, p.invert_);
};

void advGUIWindowImp::DrawRect(GUIRect &r) {
  // This is the local drawing command for the device's own screen.
  display_fill_rect(lastRemoteColorIdx, r.Left(), r.Top(), r.Width(),
                    r.Height());
  if (remoteUIEnabled_) {
    // Now, send the DrawRect command with full byte-escaping.
    // Worst-case buffer: 2 (header) + 4 * 2-byte-values * 2 (if all are
    // escaped) = 18 bytes.
    char remoteUIBuffer[20];
    auto bufferIndex = remoteUIDrawRectCommand(r.Left(), r.Top(), r.Width(),
                                               r.Height(), remoteUIBuffer);
    sendToUSBCDCBuffered(remoteUIBuffer, bufferIndex);
  }
}

void advGUIWindowImp::Clear(GUIColor &c, bool overlay) {
  color_t backgroundColor = GetColor(c);
  display_set_background(backgroundColor);
  display_clear(backgroundColor);
  if (remoteUIEnabled_) {
    char remoteUIBuffer[5];
    remoteUIClearCommand(c._r, c._g, c._b, remoteUIBuffer);
    sendToUSBCDCBuffered(remoteUIBuffer, 5); // Use the buffered function
  }
};

void advGUIWindowImp::ClearTextRect(GUIRect &r) {
  Trace::Debug("GUI ClearTextRect call");
};

color_t advGUIWindowImp::GetColor(GUIColor &c) {
  // Palette index should always be < 16. Wont check it.
  // TODO: should not be redefining the palette colors every call
  display_set_palette_color(c._paletteIndex, to_rgb565(c));
  return (color_t)c._paletteIndex;
}

void advGUIWindowImp::SetColor(GUIColor &c) {
  color_t color = GetColor(c);
  lastRemoteColorIdx = color;
  display_set_foreground(color);
  if (remoteUIEnabled_) {
    // Buffer must be large enough for the worst case where all 3 color bytes
    // are escaped. Header (2) + 3 color components * 2 bytes/escaped_component
    // = 8 bytes.
    char remoteUIBuffer[8];
    auto bufferIndex =
        remoteUISetColorCommand(c._r, c._g, c._b, remoteUIBuffer);
    sendToUSBCDCBuffered(remoteUIBuffer, bufferIndex);
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
  case SD_DET_INSERT: {
    Trace::Log("SDCARD", "Card inserted, reinitializing");
    FATFS_UnLinkDriver(SDPath);
    HAL_SD_DeInit(&hsd1);
    __HAL_RCC_SDMMC1_FORCE_RESET();
    __HAL_RCC_SDMMC1_RELEASE_RESET();
    MX_SDMMC1_SD_Init();
    FATFS_LinkDriver(&SD_DMA_Driver, SDPath);
    bool mounted =
        (f_mount(&SDFatFS, (TCHAR const *)SDPath, 0) == FR_OK) ? true : false;
    if (mounted) {
      auto fs = FileSystem::GetInstance();
      if (!fs || !fs->chdir("/")) {
        mounted = false;
      }
    }
    if (!mounted) {
      Trace::Error("Failed to mount after insert");
    }
    appwindow_set_sdcard_present(mounted);
  } break;
  case SD_DET_REMOVE: {
    Trace::Log("SDCARD", "Card removed, pausing");
    if (IsRecordingActive()) {
      StopRecording();
    }
    f_mount(NULL, (TCHAR const *)SDPath, 0);
    FATFS_UnLinkDriver(SDPath);
    HAL_SD_DeInit(&hsd1);
    appwindow_set_sdcard_present(false);
  } break;
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
