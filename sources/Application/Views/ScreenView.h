#ifndef _SCREEN_VIEW_H_
#define _SCREEN_VIEW_H_

#include "BaseClasses/View.h"
#include "ViewData.h"

class ScreenView : public View {
public:
  ScreenView(GUIWindow &w, ViewData *viewData);
  ~ScreenView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed){};
  virtual void DrawView(){};
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0){};
  virtual void OnFocus(){};
  virtual void AnimationUpdate();

private:
};
#endif
