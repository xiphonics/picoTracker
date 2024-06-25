#include "FileView.h"

FileView::FileView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {}

FileView::~FileView() {}

void FileView::ProcessButtonMask(unsigned short mask, bool pressed) {

};

void FileView::DrawView() {

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

void FileView::OnPlayerUpdate(PlayerEventType, unsigned int tick) {

};

void FileView::OnFocus() {};
