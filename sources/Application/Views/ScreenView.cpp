#include "ScreenView.h"
#include <Application/AppWindow.h>
#include <nanoprintf.h>

ScreenView::ScreenView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {}

ScreenView::~ScreenView() {}

/// Updates the animation by redrawing the battery gauge on every clock tick
void ScreenView::AnimationUpdate() {
  GUITextProperties props;
  drawBattery(props);

  // Flush the window to ensure the battery gauge is displayed
  w_.Flush();
};