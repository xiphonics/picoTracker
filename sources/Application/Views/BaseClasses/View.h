/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _VIEW_H_
#define _VIEW_H_

#include "Application/Model/Config.h"
#include "Application/Model/Project.h"
#include "Application/Player/Player.h"
#include "Application/Utils/mathutils.h"
#include "Foundation/T_SimpleList.h"
#include "I_Action.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "ViewEvent.h"
#include <functional>

#define VU_METER_HEIGHT 16
#define VU_METER_MAX 159
#define VU_METER_CLIP_LEVEL 15
#define VU_METER_WARN_LEVEL 8
#define ALT_ROW_NUMBER 4 // for now const vs a user setting

enum GUIEventPadButtonMasks {
  EPBM_LEFT = 1,
  EPBM_DOWN = 2,
  EPBM_RIGHT = 4,
  EPBM_UP = 8,
  EPBM_ALT = 16,
  EPBM_EDIT = 32,
  EPBM_ENTER = 64,
  EPBM_NAV = 128,
  EPBM_PLAY = 256,
  EPBM_SELECT = 512,
  EPBM_POWER = 1024,
};

enum ViewType {
  VT_SONG,
  VT_CHAIN,
  VT_PHRASE,
  VT_PROJECT,
  VT_DEVICE,
  VT_INSTRUMENT,
  VT_TABLE,  // Table screen under phrase
  VT_TABLE2, // Table screen under instrument
  VT_GROOVE,
  VT_MIXER,
  VT_IMPORT,            // Sample file import
  VT_INSTRUMENT_IMPORT, // Instrument file import
  VT_SELECTPROJECT,     // Select project
  VT_THEME,             // Theme settings
  VT_SELECTTHEME,       // Theme selection
  VT_THEME_IMPORT,      // Theme file import
  VT_SAMPLE_EDITOR,     // Sample Editor
  VT_RECORD             // Recording screen
};

enum ViewMode {
  VM_NORMAL,
  VM_NEW,
  VM_CLONE,
  VM_SELECTION,
  VM_MUTEON,
  VM_SOLOON
};

enum ColorDefinition {
  CD_BACKGROUND,
  CD_NORMAL,
  CD_HILITE1,
  CD_HILITE2,
  CD_CONSOLE,
  CD_CURSOR,
  CD_INFO,
  CD_WARN,
  CD_ERROR,
  CD_ACCENT,
  CD_ACCENTALT,
  CD_EMPHASIS,
  CD_RESERVED1,
  CD_RESERVED2,
  CD_RESERVED3,
  CD_RESERVED4,
};

enum ViewUpdateDirection { VUD_LEFT = 0, VUD_RIGHT, VUD_UP, VUD_DOWN };

class View;
class ModalView;

using ModalViewCallback = std::function<void(View &v, ModalView &d)>;

class View : public Observable {
public:
  View(GUIWindow &w, ViewData *viewData);
  View(View &v);

  void SetFocus(ViewType vt) {
    viewType_ = vt;
    hasFocus_ = true;
    OnFocus();
  };

  virtual void LooseFocus() {
    if (!hasFocus_) {
      return;
    }
    hasFocus_ = false;
    OnFocusLost();
  };

  void Clear();

  void ForceClear();

  void ProcessButton(unsigned short mask, bool pressed);

  void Redraw();

  bool isDirty() { return isDirty_; };

  // Override in subclasses

  virtual void DrawView() = 0;
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick) = 0;
  virtual void OnFocus() = 0;
  virtual void OnFocusLost(){};
  virtual void AnimationUpdate() = 0;

  void SetDirty(bool dirty);

  void switchToRecordView();

  // Methods to access modal view
  bool HasModalView() const { return modalView_ != nullptr; }
  ModalView *GetModalView() const { return modalView_; }

  // Primitive locking mechanism

  bool Lock();
  void WaitForObject();
  void Unlock();

  // Char based draw routines

  virtual void SetColor(ColorDefinition cd);
  virtual void ClearTextRect(int x, int y, int w, int h);
  virtual void DrawString(int x, int y, const char *txt,
                          GUITextProperties &props);
  virtual void DrawRect(GUIRect &r, ColorDefinition color);

  void DoModal(ModalView *view, ModalViewCallback cb = 0);
  void DismissModal();

protected:
  virtual void ProcessButtonMask(unsigned short mask, bool pressed) = 0;

  // to remove once everything got to viewdata

  inline void updateData(unsigned char *c, int offset, unsigned char limit,
                         bool wrap) {
    int v = *c;
    if (v == 0xFF) { // Uninitiaized data
      v = 0;
    }
    v += offset;
    if (v < 0)
      v = (wrap ? (limit + 1 + v) : 0);
    if (v > limit)
      v = (wrap ? v - (limit + 1) : limit);
    *c = v;
  }

  GUIPoint GetAnchor();
  GUIPoint GetTitlePosition();

  void drawMap();
  void drawNotes();
  void drawBattery(GUITextProperties &props);
  void drawMasterVuMeter(Player *player, GUITextProperties props,
                         bool forceRedraw = false, uint8_t xoffset = 24);
  void drawPlayTime(Player *player, GUIPoint pos, GUITextProperties &props);
  void drawVUMeter(uint8_t leftBars, uint8_t rightBars, GUIPoint pos,
                   GUITextProperties props, int vuIndex,
                   bool forceRedraw = false);
  void drawPowerButtonUI(GUITextProperties &props);

  static inline void amplitudeToBars(stereosample level, int *left,
                                     int *right) {
    // Extract both channels
    uint16_t leftAmp = (level >> 16) & 0xFFFF;
    uint16_t rightAmp = level & 0xFFFF;
    // Convert to dB
    int leftDb = amplitudeToDb(leftAmp);
    int rightDb = amplitudeToDb(rightAmp);
    // Map dB to bar levels  -60dB to 0dB range mapped to 0-159 bars
    // Optimized 159/60 ≈ 2.65 = (2.65 * 256) / 256 = 678 / 256
    // Using fixed-point: multiply by 678, then right-shift by 8 (divide by 256)
    *left = std::max(0, std::min(VU_METER_MAX, ((leftDb + 60) * 678) >> 8));
    *right = std::max(0, std::min(VU_METER_MAX, ((rightDb + 60) * 678) >> 8));
  }

public: // temp hack for modl windo constructors
  GUIWindow &w_;
  ViewData *viewData_;
  bool needsRedraw_;
  bool isVisible_;

  int vuMeterCount_;
  ViewMode viewMode_;
  bool isDirty_; // .Do we need to redraw screeen
  ViewType viewType_;
  bool hasFocus_;

  // Previous VU meter values for optimization (one pair per channel + master)
  uint8_t prevLeftVU_[SONG_CHANNEL_COUNT + 1];
  uint8_t prevRightVU_[SONG_CHANNEL_COUNT + 1];

  // Power button state
  bool powerButtonPressed_;
  int powerButtonHoldCount_;

private:
  unsigned short mask_;
  bool locked_;
  static bool initPrivate_;
  ModalView *modalView_;
  ModalViewCallback modalViewCallback_;

public:
  static int margin_;
  static int songRowCount_;
  static BatteryState batteryState_;
};

#endif
