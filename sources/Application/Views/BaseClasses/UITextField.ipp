#include "Application/AppWindow.h"
#include "View.h"
#include "stringutils.h"

template <uint8_t MaxLength>
UITextField<MaxLength>::UITextField(
    Variable &v, GUIPoint &position,
    const etl::string<MAX_UITEXTFIELD_LABEL_LENGTH> &label, uint8_t fourcc,
    etl::string<MaxLength> &defaultValue_)
    : UIField(position), src_(v), label_(label), fourcc_(fourcc),
      defaultValue_(defaultValue_) {}

template <uint8_t MaxLength> UITextField<MaxLength>::~UITextField(){};

template <uint8_t MaxLength>
void UITextField<MaxLength>::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position = GetPosition();
  position._y += offset;

  w.DrawString(label_.c_str(), position, props);
  position._x += label_.length();

  ((AppWindow &)w).SetColor(CD_INFO);

  auto value = src_.GetString().c_str();
  int len = src_.GetString().length();

  if (focus_) {
    char buffer[2];
    buffer[1] = 0;
    for (int i = 0; i < len; i++) {
      props.invert_ = (i == currentChar_);
      buffer[0] = value[i];
      w.DrawString(buffer, position, props);
      position._x += 1;
    }
  } else {
    if (len != 0) {
      w.DrawString(value, position, props);
    }
  }
};

template <uint8_t MaxLength> void UITextField<MaxLength>::OnClick() {
  SetChanged();
  NotifyObservers((I_ObservableData *)fourcc_);
};

template <uint8_t MaxLength> void UITextField<MaxLength>::OnEditClick() {
  etl::string<MAX_VARIABLE_STRING_LENGTH> buffer(src_.GetString());
  if (currentChar_ > 0 && currentChar_ < buffer.length()) {
    buffer.erase(currentChar_, 1);
    currentChar_--;
  }
  src_.SetString(buffer.c_str(), true);
  SetChanged();
  NotifyObservers((I_ObservableData *)fourcc_);
};

template <uint8_t MaxLength>
void UITextField<MaxLength>::ProcessArrow(unsigned short mask) {
  auto buffer(src_.GetString());

  switch (mask) {
  case EPBM_UP:
    // on a char edit key (up or down), if we are on the default string value,
    // we want to clear default value and start user off with just first char
    if (buffer.compare(defaultValue_) == 0) {
      currentChar_ = 0;
      buffer.clear();
      buffer.append("A");
      src_.SetString(buffer.c_str(), true);
    } else {
      buffer[currentChar_] = getNext(buffer.c_str()[currentChar_], false);
      src_.SetString(buffer.c_str(), true);
    }
    SetChanged();
    NotifyObservers((I_ObservableData *)fourcc_);
    break;
  case EPBM_DOWN:
    if (buffer.compare(defaultValue_) == 0) {
      currentChar_ = 0;
      buffer.clear();
      buffer.append("A");
      src_.SetString(buffer.c_str(), true);
    }
    buffer[currentChar_] = getNext((char)buffer.c_str()[currentChar_], true);
    src_.SetString(buffer.c_str(), true);
    SetChanged();
    NotifyObservers((I_ObservableData *)fourcc_);
    break;
  case EPBM_LEFT:
    if (currentChar_ > 0) {
      currentChar_--;
    }
    break;
  case EPBM_RIGHT:
    if (currentChar_ < (buffer.length() - 1)) {
      currentChar_++;
      // -1 to allow for adding 1 more char
    } else if (currentChar_ < (MaxLength - 1)) {
      currentChar_++;
      buffer.append("A");
      src_.SetString(buffer.c_str(), true);
      SetChanged();
      NotifyObservers((I_ObservableData *)fourcc_);
    }
    break;
  };
};

template <uint8_t MaxLength>
etl::string<MaxLength> UITextField<MaxLength>::GetString() {
  return src_.GetString().substr(0, MaxLength);
};
