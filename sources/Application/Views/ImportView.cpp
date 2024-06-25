#include "ImportView.h"

ImportView::ImportView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {}

ImportView::~ImportView() {}

void ImportView::ProcessButtonMask(unsigned short mask, bool pressed) {

};

void ImportView::DrawView() {

  Clear();

  GUITextProperties props;
  SetColor(CD_HILITE2);

  char buildString[80];
  sprintf(buildString, "picoTracker build %s%s_%s", PROJECT_NUMBER,
          PROJECT_RELEASE, BUILD_COUNT);
  GUIPoint pos;
  pos._y = 22;
  pos._x = (32 - strlen(buildString)) / 2;
  DrawString(pos._x, pos._y, buildString, props);
};

void ImportView::OnPlayerUpdate(PlayerEventType, unsigned int tick) {

};

void ImportView::OnFocus() {};
