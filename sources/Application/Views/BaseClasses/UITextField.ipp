#include "Application/AppWindow.h"
#include "View.h"
#include "stringutils.h"

template <uint8_t MaxLength>
UITextField<MaxLength>::UITextField(Variable &v, GUIPoint &position,
                                    const etl::string<MAX_LABEL_LENGTH> &label,
                                    uint8_t fourcc,
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
  if (len == 0) {
    value = defaultValue_.c_str();
    len = defaultValue_.length();
  }
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
    w.DrawString(value, position, props);
  }
};

template <uint8_t MaxLength> void UITextField<MaxLength>::OnClick() {
  currentChar_ = 0;
  SetChanged();
  NotifyObservers((I_ObservableData *)fourcc_);
};

template <uint8_t MaxLength> void UITextField<MaxLength>::OnEditClick() {
  char name[MaxLength + 1];
  strcpy(name, src_.GetString().c_str());
  uint8_t len = std::strlen(name);
  deleteChar(name, currentChar_);
  if (currentChar_ && (currentChar_ == len - 1))
    currentChar_--;
  src_.SetString(name, true);
  SetChanged();
  NotifyObservers((I_ObservableData *)fourcc_);
};

template <uint8_t MaxLength>
void UITextField<MaxLength>::ProcessArrow(unsigned short mask) {
  char name[MaxLength + 1];
  strcpy(name, src_.GetString().c_str());
  int len = std::strlen(name);

  switch (mask) {
  case EPBM_UP:
    if (!strcmp(name, defaultValue_.c_str())) {
      currentChar_ = 0;
      name[0] = 'A' - 1; // to land in A
      name[1] = '\0';
    }
    name[currentChar_] = getNext(name[currentChar_], false);
    src_.SetString(name, true);
    SetChanged();
    NotifyObservers((I_ObservableData *)fourcc_);
    break;
  case EPBM_DOWN:
    if (!strcmp(name, defaultValue_.c_str())) {
      currentChar_ = 0;
      name[0] = 'A' + 1; // to land in A
      name[1] = '\0';
    }
    name[currentChar_] = getNext(name[currentChar_], true);
    src_.SetString(name, true);
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
    } else if (currentChar_ < MaxLength - 1) {
      currentChar_++;
      name[currentChar_] = 'A';
      name[currentChar_ + 1] = '\0';
      len++;
      src_.SetString(name, true);
      SetChanged();
      NotifyObservers((I_ObservableData *)fourcc_);
    }
    break;
  };
};

template <uint8_t MaxLength>
etl::string<MaxLength> UITextField<MaxLength>::GetString() {
  return src_.GetString();
};
