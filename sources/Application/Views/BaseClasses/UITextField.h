#ifndef _UI_TEXT_FIELD_
#define _UI_TEXT_FIELD_

#include "Foundation/Observable.h"
#include "UIField.h"

class UITextField : public UIField, public Observable {
public:
  UITextField(char *name, GUIPoint &position);

  virtual ~UITextField();
  void Draw(GUIWindow &w, int offset = 0);
  void ProcessArrow(unsigned short mask);
  void OnClick();
  const char *GetString();

private:
  int selected_;
  int lastChar_;
  char name_[24];
  int currentChar_ = 0;
  int len_ = 0;
};
#endif
