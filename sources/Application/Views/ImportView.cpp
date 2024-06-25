#include "ImportView.h"

ImportView::ImportView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {}

ImportView::~ImportView() {}

void ImportView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (mask & EPBM_LEFT) {
    // Go to import sample
    ViewType vt = VT_INSTRUMENT;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
  }
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
