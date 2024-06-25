#ifndef _IMPORT_VIEW_H_
#define _IMPORT_VIEW_H_

#include "BaseClasses/View.h"
#include "ViewData.h"

class ImportView : public View {
public:
  ImportView(GUIWindow &w, ViewData *viewData);
  ~ImportView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();
  virtual void AnimationUpdate() {};

private:
};
#endif
