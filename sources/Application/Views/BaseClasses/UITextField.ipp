#include "Application/AppWindow.h"
#include "View.h"
#include "Application/Utils/stringutils.h"

template <uint8_t MaxLength>
UITextField<MaxLength>::UITextField(
    Variable &v, const GUIPoint &position,
    const etl::string<MAX_UITEXTFIELD_LABEL_LENGTH> &label, uint8_t fourcc,
    etl::string<MaxLength> &defaultValue_)
    : UIField(position), src_(&v), label_(label), fourcc_(fourcc),
      defaultValue_(defaultValue_) {}

template <uint8_t MaxLength> UITextField<MaxLength>::~UITextField(){};

template <uint8_t MaxLength>
void UITextField<MaxLength>::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position = GetPosition();
  position._y += offset;

  ((AppWindow &)w).SetColor(CD_NORMAL);
  w.DrawString(label_.c_str(), position, props);
  position._x += label_.length();

  ((AppWindow &)w).SetColor(CD_EMPHASIS);

  auto srcString = src_->GetString();
  const char *value;
  int len;

  // If the variable's value is empty, use the default value for display
  if (srcString.empty()) {
    value = defaultValue_.c_str();
    len = defaultValue_.length();
    // Use a different color for default values to indicate they're not set
    ((AppWindow &)w).SetColor(CD_EMPHASIS);
  } else {
    value = srcString.c_str();
    len = srcString.length();
  }

  if (focus_) {
    if (len == 0) {
      // For empty fields, draw a cursor at the beginning position
      props.invert_ = true;
      w.DrawString(" ", position, props);
    } else {
      char buffer[2];
      buffer[1] = 0;
      for (int i = 0; i < len; i++) {
        props.invert_ = (i == currentChar_);
        buffer[0] = value[i];
        w.DrawString(buffer, position, props);
        position._x += 1;
      }
    }
  } else {
    if (len != 0) {
      w.DrawString(value, position, props);
    }
  }
};

template <uint8_t MaxLength> void UITextField<MaxLength>::OnClick() {
  SetChanged();
  NotifyObservers(
      reinterpret_cast<I_ObservableData *>(static_cast<uintptr_t>(fourcc_)));
};

template <uint8_t MaxLength> void UITextField<MaxLength>::OnEditClick() {
  etl::string<MAX_VARIABLE_STRING_LENGTH> buffer(src_->GetString());
  if (currentChar_ > 0 && currentChar_ < buffer.length()) {
    buffer.erase(currentChar_, 1);
    currentChar_--;
  }
  src_->SetString(buffer.c_str(), true);
  SetChanged();
  NotifyObservers(
      reinterpret_cast<I_ObservableData *>(static_cast<uintptr_t>(fourcc_)));
};

template <uint8_t MaxLength>
void UITextField<MaxLength>::ProcessClear() {
  etl::string<MAX_VARIABLE_STRING_LENGTH> buffer(src_->GetString());

  if (buffer.empty() || buffer.compare(defaultValue_) == 0) {
    // handle empty state
    buffer = "A";  
    currentChar_ = 0;
  } else {
    // jump to lastUsedChar_character block depending on the current character
    if (lastUsedChar_>= 'A' && lastUsedChar_<= 'Z') {
      lastUsedChar_= 'a';
    } else if (lastUsedChar_>= 'a' && lastUsedChar_<= 'z') {
      lastUsedChar_= '0';
    } else if (lastUsedChar_>= '0' && lastUsedChar_<= '9') {
      lastUsedChar_= '-';
    } else {
      lastUsedChar_= 'A';
    }
  }

  buffer[currentChar_] = lastUsedChar_;
  src_->SetString(buffer.c_str(), true);
  SetChanged();
  NotifyObservers(
      reinterpret_cast<I_ObservableData *>(static_cast<uintptr_t>(fourcc_)));
};

template <uint8_t MaxLength>
void UITextField<MaxLength>::ProcessArrow(unsigned short mask) {
  etl::string<MAX_VARIABLE_STRING_LENGTH> buffer(src_->GetString());
  auto applyAndNotify = [&]() {
    src_->SetString(buffer.c_str(), true);
    SetChanged();
    NotifyObservers(
        reinterpret_cast<I_ObservableData *>(static_cast<uintptr_t>(fourcc_)));
  };

  // If the variable's value is empty, we need to initialize it when the user
  // starts editing
  bool isEmptyBuffer = buffer.empty();

  switch (mask) {
  case EPBM_UP:
  case EPBM_DOWN:
    // If buffer is empty or matches default, initialize with 'A'
    if (isEmptyBuffer || buffer.compare(defaultValue_) == 0) {
      currentChar_ = 0;
      buffer = "A";
    } else {
      buffer[currentChar_] = 
        getNext(buffer.c_str()[currentChar_], mask == EPBM_DOWN);
    }
    applyAndNotify();
    break;
  case EPBM_LEFT:
    // If we're showing the default value and user presses left, initialize with
    // the default
    if (isEmptyBuffer) {
      buffer = defaultValue_;
      applyAndNotify();
    }
    if (currentChar_ > 0) {
      currentChar_--;
    }
    break;
  case EPBM_RIGHT:
    // If we're showing the default value and user presses right, initialize
    // with the default
    if (isEmptyBuffer) {
      buffer = defaultValue_;
      applyAndNotify();
    }
    if (currentChar_ < (buffer.length() - 1)) {
      currentChar_++;
      // -1 to allow for adding 1 more char
    } else if (currentChar_ < (MaxLength - 1)) {
      currentChar_++;
      char str[2] = {lastUsedChar_, 0};
      buffer.append(str);
      applyAndNotify();
    }
    break;
  };

  // remember last used char for appending when user moves right at the end
  // of the string
  lastUsedChar_= buffer.c_str()[currentChar_];
};

template <uint8_t MaxLength>
etl::string<MaxLength> UITextField<MaxLength>::GetString() {
  return src_->GetString().substr(0, MaxLength);
};

template <uint8_t MaxLength>
void UITextField<MaxLength>::SetVariable(Variable &v) {
  // Set the variable this UITextField is bound to
  src_ = &v;
  currentChar_ = 0; // Reset cursor position
};
