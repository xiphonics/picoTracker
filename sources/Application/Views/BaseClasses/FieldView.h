#ifndef _FIELD_VIEW_H_
#define _FIELD_VIEW_H_

#include "View.h"
#include "Foundation/T_SimpleList.h"
#include "UIField.h"

class FieldView: public View,public T_SimpleList<UIField> {
public:
	FieldView(GUIWindow &w,ViewData *viewData) ;

	virtual void Redraw() ;
	virtual void ProcessButtonMask(unsigned short mask) ;

	void SetFocus(UIField *) ;
	UIField *GetFocus() ;
	void ClearFocus() ;
	int GetFocusIndex() ;
	void SetSize(int size) ;
private:
	T_SimpleList<UIField> fieldList_ ;
	UIField *focus_ ;
} ;

#endif
