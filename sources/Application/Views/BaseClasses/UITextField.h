#ifndef _UI_TEXT_FIELD_
#define _UI_TEXT_FIELD_

#include "Foundation/Observable.h"
#include "UIField.h"

class UITextField : public UIField, public Observable {
public:
  UITextField(Variable *v, GUIPoint &position, const char *name,
              unsigned int fourcc);

  virtual ~UITextField();
  void Draw(GUIWindow &w, int offset = 0);
  void ProcessArrow(unsigned short mask);
  void OnClick();
  void OnBClick();
  etl::string<40> GetString();

private:
  int selected_;
  int currentChar_ = 0;
  Variable *src_;
  const char *name_;
  unsigned int fourcc_;
};

char getNext(char c, bool reverse);
void deleteChar(char *name, uint8_t pos);
#endif
