
#include "UIIntVarOffField.h"
#include "Application/AppWindow.h"
#include <System/Console/nanoprintf.h>

UIIntVarOffField::UIIntVarOffField(GUIPoint &position, Variable &v,
                                   const char *format, int min, int max,
                                   int xOffset, int yOffset)
    : UIIntVarField(position, v, format, min, max, xOffset, yOffset) {}

void UIIntVarOffField::ProcessArrow(unsigned short mask) {

  int value = src_.GetInt();

  if (value == VAR_OFF) { // Off state
    switch (mask) {
    case EPBM_UP:
      value = min_ + yOffset_;
      break;
    case EPBM_RIGHT:
      value = min_;
      break;
    };
  } else {
    switch (mask) {
    case EPBM_UP:
      value += yOffset_;
      break;
    case EPBM_DOWN:
      value -= yOffset_;
      break;
    case EPBM_LEFT:
      value -= xOffset_;
      break;
    case EPBM_RIGHT:
      value += xOffset_;
      break;
    };
    if (value < min_) {
      value = VAR_OFF;
    };
    if (value > max_) {
      value = max_;
    }
  }
  src_.SetInt(value);
};

void UIIntVarOffField::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position = GetPosition();

  if (focus_) {
    ((AppWindow &)w).SetColor(CD_HILITE2);
    props.invert_ = true;
  } else {
    ((AppWindow &)w).SetColor(CD_NORMAL);
  }
  Variable::Type type = src_.GetType();
  char buffer[MAX_FIELD_WIDTH + 1];
  switch (type) {
  case Variable::INT: {
    int ivalue = src_.GetInt();
    if (ivalue != VAR_OFF) {
      npf_snprintf(buffer, sizeof(buffer), format_, ivalue, ivalue);
    } else {
      char format[64];
      strcpy(format, format_);
      char *location = strchr(format, '%');
      if (location) {
        while ((*location != 0) && (*location != 'x') && (*location != 'X') &&
               (*location != 'd')) {
          location++;
        }
        if (*location != 0) {
          *location = 's';
        } else {
          location = 0;
        }
      }
      if (location) {
        npf_snprintf(buffer, sizeof(buffer), format, "--");
      } else {
        strcpy(buffer, "++wtf++");
      }
    }
  } break;

  default:
    strcpy(buffer, "++wtf++");
  }
  w.DrawString(buffer, position, props);
};
