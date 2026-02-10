/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _APP_WINDOW_H_
#define _APP_WINDOW_H_

#include "Application/Views/BaseClasses/View.h"
#include "Application/Views/ViewData.h"
#include "Foundation/Observable.h"
#include "System/Process/SysMutex.h"
#include "System/io/Status.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include <UIFramework/SimpleBaseClasses/EventManager.h>
#include <cstddef>
#include <new>
#include <type_traits>

#define PROP_INVERT 0x80
#ifdef ADV
#define CHAR_WIDTH 22
#define CHAR_HEIGHT 30
#else
#define CHAR_WIDTH 10
#define CHAR_HEIGHT 10
#endif
#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 24
#define SCREEN_MAP_HEIGHT 4
#define SCREEN_MAP_WIDTH 4
#define BATTERY_GAUGE_WIDTH 5
#define SCREEN_CHARS SCREEN_WIDTH *SCREEN_HEIGHT
#define MAX_FIELD_WIDTH 26
#define SCREEN_REDRAW_RATE PICO_CLOCK_HZ

class View;
struct AppWindowViews;

class AppWindow : public GUIWindow, I_Observer, Status {
protected:
  AppWindow(I_GUIWindowImp &imp, const char *projectName);
  virtual ~AppWindow();

public:
  static AppWindow *Create(GUICreateWindowParams &, const char *projectName);

  static GUIColor GetColor(ColorDefinition cd);

  enum LoadProjectResult { LOAD_FAILED = -1, LOAD_OK = 0 };

  LoadProjectResult LoadProject(const char *name);
  void CloseProject();

  using GUIWindow::Clear;
  virtual void Clear(bool all = false);
  virtual void ClearTextRect(GUIRect &rect);
  virtual void SetColor(ColorDefinition cd);

  void SetDirty();
  void UpdateColorsFromConfig();
  void SetSdCardPresent(bool present);

  char projectName_[MAX_PROJECT_NAME_LENGTH + 1];

protected: // GUIWindow implementation
  virtual bool onEvent(GUIEvent &event);
  virtual void onUpdate(bool redraw);
  virtual void LayoutChildren();
  virtual void Flush();
  virtual void Redraw(){};
  virtual void AnimationUpdate();

  // override draw string to avoid going too far off
  // the screen.
  virtual void DrawString(const char *string, const GUIPoint &pos,
                          const GUITextProperties &props, bool overlay = false);

  // I_Observer implementation

  virtual void Update(Observable &o, I_ObservableData *d);

  // Status implementation

  virtual void Print(char *);
  virtual void PrintMultiLine(char *);

  void defineColor(FourCC colorCode, GUIColor &color, int paletteIndex);

  void onQuitApp();

private:
  bool AutoSave();

  Project project_;
  ViewData viewData_;
  AppWindowViews *views_;
  View *_currentView;

  bool _closeProject;
  bool _shouldQuit;
  unsigned short _mask;
  unsigned long _lastA;
  unsigned long _lastB;
  char _statusLine[80];

  bool lowBatteryState_;
  bool lowBatteryMessageShown_;
  uint16_t lowBatteryWarningCounter_;
  bool sdCardMissing_;
  bool sdCardMessageShown_;

  static unsigned char _charScreen[SCREEN_CHARS];
  static unsigned char _charScreenProp[SCREEN_CHARS];
  static unsigned char _preScreen[SCREEN_CHARS];
  static unsigned char _preScreenProp[SCREEN_CHARS];

  static GUIColor backgroundColor_;
  static GUIColor normalColor_;
  static GUIColor highlightColor_;
  static GUIColor highlight2Color_;
  static GUIColor consoleColor_;
  static GUIColor cursorColor_;
  static GUIColor infoColor_;
  static GUIColor warnColor_;
  static GUIColor errorColor_;
  static GUIColor accentColor_;
  static GUIColor accentAltColor_;
  static GUIColor emphasisColor_;
  static GUIColor reserved1Color_;
  static GUIColor reserved2Color_;
  static GUIColor reserved3Color_;
  static GUIColor reserved4Color_;

  ColorDefinition colorIndex_;

  static int charWidth_;
  static int charHeight_;

  bool loadProject_ = false;
  bool awaitingProjectLoadAck_ = false;
  bool createProjectOnLoad_ = false;
  bool playerInitialized_ = false;

  uint32_t lastAutoSave = 0;

  // Counter for animation frames, updated once per frame at PICO_CLOCK_HZ
  static uint32_t animationFrameCounter_;

public:
  // Static accessor for the animation frame counter
  static uint32_t GetAnimationFrameCounter() { return animationFrameCounter_; }

  // Internal modal callback API used by View to keep lambda callbacks alive
  // until modal dismissal.
  void StoreModalCallback(const void *source, size_t size, size_t align,
                          void (*copyFn)(void *, const void *),
                          void (*destroyFn)(void *),
                          void (*invokeFn)(void *, View &, ModalView &));
  void InvokeModalCallback(View &v, ModalView &d);
  void ClearModalCallback();

private:
  struct ModalCallbackSlot {
    static constexpr size_t StorageSize = 64;
    using CopyFn = void (*)(void *, const void *);
    using DestroyFn = void (*)(void *);
    using InvokeFn = void (*)(void *, View &, ModalView &);

    void Store(const void *source, size_t size, size_t align, CopyFn copyFn,
               DestroyFn destroyFn, InvokeFn invokeFn) {
      Clear();
      if (size > StorageSize || align > alignof(std::max_align_t) || !source ||
          !copyFn || !destroyFn || !invokeFn) {
        return;
      }
      copyFn(storage_, source);
      invoke_ = invokeFn;
      destroy_ = destroyFn;
      active_ = true;
    }

    void Invoke(View &v, ModalView &d) {
      if (active_ && invoke_) {
        invoke_(storage_, v, d);
      }
    }

    void Clear() {
      if (active_ && destroy_) {
        destroy_(storage_);
      }
      active_ = false;
      invoke_ = nullptr;
      destroy_ = nullptr;
    }

    alignas(std::max_align_t) unsigned char storage_[StorageSize];
    InvokeFn invoke_ = nullptr;
    DestroyFn destroy_ = nullptr;
    bool active_ = false;
  };

  ModalCallbackSlot modalCallbackSlot_;
};

#endif
