#ifndef _UI_TEXT_FIELD_
#define _UI_TEXT_FIELD_

#include "Foundation/Observable.h"
#include "UIField.h"

class UITextField : public UIField, public Observable {
public:
  UITextField(Variable *v, GUIPoint &position, const char *name);

  virtual ~UITextField();
  void Draw(GUIWindow &w, int offset = 0);
  void ProcessArrow(unsigned short mask);
  void OnClick();
  etl::string<40> GetString();

private:
  int selected_;
  int lastChar_;
  int currentChar_ = 0;
  Variable *src_;
  const char *name_;
};
#endif
