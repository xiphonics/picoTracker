#pragma once

#include "Application/AppWindow.h"
#include "Application/Views/BaseClasses/View.h"
#include "System/System/System.h"
#include <string.h>

constexpr int maxLineWidth = (SCREEN_WIDTH - 6);
constexpr int maxLines = 8;

struct ToastType {
  const char *symbol;
  ColorDefinition color;
};

constexpr ToastType ttInfo = {"i", CD_INFO};
constexpr ToastType ttError = {"X", CD_ERROR};
constexpr ToastType ttSuccess = {"I", CD_HILITE1};
constexpr ToastType ttWarning = {"!", CD_WARN};

class ToastView : public View {
public:
  virtual ~ToastView();

  static void Init(GUIWindow &w, ViewData *viewData);
  static ToastView *getInstance();

  void Show(const char *text, const ToastType *type, uint32_t msTime);
  void Draw(GUIWindow &w, ViewData *viewData);
  void UpdateTimer();

private:
  static ToastView *instance_;
  char lines_[maxLines][SCREEN_WIDTH + 1];
  ToastType type_ = ttInfo;
  uint32_t dismissTime_ = 0;
  uint32_t animationStartTime_ = 0;
  int32_t lineCount_ = 0;
  int animationOffset_ = 0;
  bool visible_ = false;

  ToastView(GUIWindow &w, ViewData *viewData);
  void WrapText(const char *message);

  // view virtual methods
  virtual void OnFocus() override{};
  virtual void DrawView() override{};
  virtual void AnimationUpdate() override{};
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick) override{};
  virtual void ProcessButtonMask(unsigned short mask, bool pressed) override{};
};