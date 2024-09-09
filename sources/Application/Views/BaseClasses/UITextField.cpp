#include "UITextField.h"
#include "Application/AppWindow.h"
#include "View.h"

UITextField::UITextField(Variable *v, GUIPoint &position, const char *name,
                         unsigned int fourcc)
    : UIField(position), src_(v) {
  name_ = name;
  fourcc_ = fourcc;
};

UITextField::~UITextField(){};

void UITextField::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position = GetPosition();
  position._y += offset;

  w.DrawString(name_, position, props);
  position._x += strlen(name_);

  ((AppWindow &)w).SetColor(CD_INFO);

  auto name = src_->GetString().c_str();
  int len = strlen(name);
  if (focus_) {
    char buffer[2];
    buffer[1] = 0;
    for (int i = 0; i < len; i++) {
      props.invert_ = (i == currentChar_);
      buffer[0] = name[i];
      w.DrawString(buffer, position, props);
      position._x += 1;
    }
  } else {
    w.DrawString(name, position, props);
  }
};

void UITextField::OnClick(){
    // NotifyObservers((I_ObservableData *)fourcc_);
};

void UITextField::ProcessArrow(unsigned short mask) {
  char name[40];
  strcpy(name, src_->GetString().c_str());
  int len = strlen(name);

  switch (mask) {
  case EPBM_UP:
    name[currentChar_] += 1;
    lastChar_ = name[currentChar_];
    src_->SetString(name, true);
    SetChanged();
    NotifyObservers((I_ObservableData *)fourcc_);
    break;
  case EPBM_DOWN:
    name[currentChar_] -= 1;
    lastChar_ = name[currentChar_];
    src_->SetString(name, true);
    SetChanged();
    NotifyObservers((I_ObservableData *)fourcc_);
    break;
  case EPBM_LEFT:
    if (currentChar_ > 0) {
      currentChar_--;
    }
    break;
  case EPBM_RIGHT:
    if (currentChar_ < len - 1) {
      currentChar_++;
    }
    break;
  };
};

etl::string<40> UITextField::GetString() { return src_->GetString(); };
