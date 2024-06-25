#ifndef _NULL_VIEW_H_
#define _NULL_VIEW_H_

#include "BaseClasses/View.h"
#include "ViewData.h"

class FileView : public View {
public:
  FileView(GUIWindow &w, ViewData *viewData);
  ~FileView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();
  virtual void AnimationUpdate(){};

private:
};
#endif
