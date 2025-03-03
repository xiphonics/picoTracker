#ifndef _UI_TEXT_FIELD_
#define _UI_TEXT_FIELD_

#include "Foundation/Observable.h"
#include "UIField.h"
#include "stdint.h"

#define MAX_LABEL_LENGTH 8

template <uint8_t MaxLength>
class UITextField : public UIField, public Observable {
public:
  UITextField(Variable &v, GUIPoint &position,
              const etl::string<MAX_LABEL_LENGTH> &label, uint8_t fourcc,
              etl::string<MaxLength> &defaultValue_);

  virtual ~UITextField();
  void Draw(GUIWindow &w, int offset = 0);
  void ProcessArrow(unsigned short mask);
  void OnClick();
  void OnEditClick();
  etl::string<MaxLength> GetString();

private:
  int selected_;
  int currentChar_ = 0;
  Variable &src_;
  const etl::string<8> label_;
  uint8_t fourcc_;
  etl::string<MaxLength> defaultValue_;
};

#include "UITextField.ipp"

#endif
