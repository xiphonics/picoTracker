#include "UITextField.h"
#include "Application/AppWindow.h"
#include "View.h"

UITextField::UITextField(char *name, GUIPoint &position) : UIField(position) {
  strcpy(name_, name);
  len_ = (int)strlen(name_);
};

UITextField::~UITextField(){};

void UITextField::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position = GetPosition();
  position._y += offset;

  ((AppWindow &)w).SetColor(CD_INFO);

  if (focus_) {
    char buffer[2];
    buffer[1] = 0;
    for (int i = 0; i < len_; i++) {
      props.invert_ = (i == currentChar_);
      buffer[0] = name_[i];
      w.DrawString(buffer, position, props);
      position._x += 1;
    }
  } else {
    w.DrawString(name_, position, props);
  }
};

void UITextField::OnClick() {
  SetChanged();
  // NotifyObservers((I_ObservableData *)fourcc_);
};

void UITextField::ProcessArrow(unsigned short mask) {

  switch (mask) {
  case EPBM_UP:
    name_[currentChar_] += 1;
    lastChar_ = name_[currentChar_];
    break;
  case EPBM_DOWN:
    name_[currentChar_] -= 1;
    lastChar_ = name_[currentChar_];
    break;
  case EPBM_LEFT:
    if (currentChar_ > 0) {
      currentChar_--;
    }
    break;
  case EPBM_RIGHT:
    if (currentChar_ < len_ - 1) {
      currentChar_++;
    }
    break;
  };
};

const char *UITextField::GetString() { return name_; };
