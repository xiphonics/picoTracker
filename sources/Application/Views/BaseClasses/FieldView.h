#ifndef _FIELD_VIEW_H_
#define _FIELD_VIEW_H_

#include "Foundation/T_SimpleList.h"
#include "UIField.h"
#include "View.h"

class FieldView : public View {
public:
  FieldView(GUIWindow &w, ViewData *viewData);

  virtual void Redraw();
  virtual void ProcessButtonMask(unsigned short mask);

  void SetFocus(UIField *);
  UIField *GetFocus();
  void ClearFocus();
  int GetFocusIndex();
  void SetSize(int size);

  etl::list<UIField *, 26> fieldList_; // adjust to maximum fields on one screen

private:
  UIField *focus_;
};

#endif
