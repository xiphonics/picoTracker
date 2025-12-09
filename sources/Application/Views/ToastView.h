#pragma once

#include "Application/AppWindow.h"
#include "Application/Views/BaseClasses/View.h"
#include "System/System/System.h"
#include <string.h>

enum ToastType {
  TT_INFO,
  TT_SUCCESS,
  TT_WARNING,
  TT_ERROR
};

class ToastView : public View {
public:
  ToastView(GUIWindow &w, ViewData *viewData);
  virtual ~ToastView();

  // Static methods for convenience
  void UpdateTimer();
  
  static ToastView* instance_;
  
  static ToastView *getInstance() {
    return instance_;
  }

  #define TOAST_MAX_LINE_WIDTH (SCREEN_WIDTH - 6)

  char *lines_[8]; 
  int lineCount_;
  uint32_t dismissTime_;
  bool visible_;
  ToastType type_;
  
  void Show(const char *message, ToastType type, uint32_t timeout_ms);
  void Draw(GUIWindow &w, ViewData *viewData);
  void WrapText(const char *message);
  char GetTypeIcon();
  
  // View virtual methods
  virtual void ProcessButtonMask(unsigned short mask, bool pressed) override {};
  virtual void DrawView() override {};
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0) override {};
  virtual void OnFocus() override {};
  virtual void AnimationUpdate() override {};
};