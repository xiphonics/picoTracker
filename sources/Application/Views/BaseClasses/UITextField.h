#ifndef _UI_TEXT_FIELD_
#define _UI_TEXT_FIELD_

#include "Foundation/Observable.h"
#include "UIField.h"
#include "stdint.h"

template <uint8_t MaxLength>
class UITextField : public UIField, public Observable {
public:
  UITextField(Variable *v, GUIPoint &position, const char *name, uint8_t fourcc,
              const char *defaultValue_);

  virtual ~UITextField();
  void Draw(GUIWindow &w, int offset = 0);
  void ProcessArrow(unsigned short mask);
  void OnClick();
  void OnEditClick();
  etl::string<MaxLength> GetString();

private:
  int selected_;
  int currentChar_ = 0;
  Variable *src_;
  const char *name_;
  uint8_t fourcc_;
  const char *defaultValue_;
};

#include "UITextField.ipp"

#endif
