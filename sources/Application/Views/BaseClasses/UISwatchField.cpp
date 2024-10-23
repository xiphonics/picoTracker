#include "UISwatchField.h"
#include "Application/AppWindow.h"

UISwatchField::UISwatchField(GUIPoint &position, const ColorDefinition color)
    : UIField(position) {
  color_ = color;
};

void UISwatchField::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position = GetPosition();
  position._y += offset;

  ((AppWindow &)w).SetColor(color_);
  w.DrawString("####", position, props);
  ((AppWindow &)w).SetColor(CD_NORMAL);
};

void UISwatchField::ProcessArrow(unsigned short mask){};

bool UISwatchField::IsStatic() { return true; };
