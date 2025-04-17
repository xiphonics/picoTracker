#ifndef _FIELD_VIEW_H_
#define _FIELD_VIEW_H_

#include "BaseClasses/UIField.h"
#include "Foundation/T_SimpleList.h"
#include "ScreenView.h"

class FieldView : public ScreenView {
public:
  FieldView(GUIWindow &w, ViewData *viewData);

  virtual void Redraw();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed) override;

  void SetFocus(UIField *);
  UIField *GetFocus();
  void ClearFocus();
  int GetFocusIndex();
  void SetSize(int size);

  etl::list<UIField *, 35> fieldList_; // adjust to maximum fields on one screen

private:
  UIField *focus_;
};

#endif
