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

void UITextField::OnClick() {
  currentChar_ = 0;
  SetChanged();
  NotifyObservers((I_ObservableData *)fourcc_);
};

void UITextField::OnEditClick() {
  char name[MAX_PROJECT_NAME_LENGTH + 1];
  strcpy(name, src_->GetString().c_str());
  uint8_t len = std::strlen(name);
  deleteChar(name, currentChar_);
  if (currentChar_ && (currentChar_ == len - 1))
    currentChar_--;
  src_->SetString(name, true);
  SetChanged();
  NotifyObservers((I_ObservableData *)fourcc_);
};

void UITextField::ProcessArrow(unsigned short mask) {
  char name[MAX_PROJECT_NAME_LENGTH + 1];
  strcpy(name, src_->GetString().c_str());
  int len = std::strlen(name);

  switch (mask) {
  case EPBM_UP:
    if (!strcmp(name, UNNAMED_PROJECT_NAME)) {
      currentChar_ = 0;
      name[0] = 'A' - 1; // to land in A
      name[1] = '\0';
    }
    name[currentChar_] = getNext(name[currentChar_], false);
    src_->SetString(name, true);
    SetChanged();
    NotifyObservers((I_ObservableData *)fourcc_);
    break;
  case EPBM_DOWN:
    if (!strcmp(name, UNNAMED_PROJECT_NAME)) {
      currentChar_ = 0;
      name[0] = 'A' + 1; // to land in A
      name[1] = '\0';
    }
    name[currentChar_] = getNext(name[currentChar_], true);
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
    } else if (currentChar_ < MAX_PROJECT_NAME_LENGTH - 1) {
      currentChar_++;
      name[currentChar_] = 'A';
      name[currentChar_ + 1] = '\0';
      len++;
      src_->SetString(name, true);
      SetChanged();
      NotifyObservers((I_ObservableData *)fourcc_);
    }
    break;
  };
};

etl::string<MAX_PROJECT_NAME_LENGTH> UITextField::GetString() {
  return src_->GetString();
};

char getNext(char c, bool reverse) {
  // Valid characters in order
  const char validChars[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._";
  const int numChars = sizeof(validChars) - 1; // Exclude null terminator

  // Find the index of the current character
  for (int i = 0; i < numChars; ++i) {
    if (validChars[i] == c) {
      // Calculate next index based on direction (forward or reverse)
      int nextIndex =
          reverse ? (i - 1 + numChars) % numChars : (i + 1) % numChars;
      return validChars[nextIndex];
    }
  }

  // If character is not valid, return the first valid character in the list
  return reverse ? validChars[numChars - 1] : validChars[0];
};

void deleteChar(char *name, uint8_t pos) {
  int len = std::strlen(name);

  // If length is 1 or position is invalid, do nothing
  if (len <= 1 || pos < 0 || pos >= len) {
    return;
  }

  // Shift characters left starting from the position
  for (int i = pos; i < len - 1; ++i) {
    name[i] = name[i + 1];
  }

  // Null-terminate the string at the new end
  name[len - 1] = '\0';
}
